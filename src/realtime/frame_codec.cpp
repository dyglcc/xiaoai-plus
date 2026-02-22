#include "realtime/frame_codec.hpp"

#include <arpa/inet.h>
#include <zlib.h>

#include <cstring>
#include <stdexcept>

#include <spdlog/spdlog.h>

namespace xiaoai_plus::realtime::protocol {

namespace {

void WriteU32(std::vector<uint8_t>* out, uint32_t v) {
  uint32_t be = htonl(v);
  const auto* p = reinterpret_cast<const uint8_t*>(&be);
  out->insert(out->end(), p, p + sizeof(uint32_t));
}

uint32_t ReadU32(const std::vector<uint8_t>& data, size_t* offset) {
  if (*offset + 4 > data.size()) {
    throw std::runtime_error("frame decode: truncated uint32");
  }
  uint32_t be = 0;
  std::memcpy(&be, data.data() + *offset, 4);
  *offset += 4;
  return ntohl(be);
}

int32_t ReadI32(const std::vector<uint8_t>& data, size_t* offset) {
  return static_cast<int32_t>(ReadU32(data, offset));
}

bool HasEvent(uint8_t flags) { return (flags & 0x04) == 0x04; }

bool HasSequence(uint8_t flags) {
  const uint8_t sequence_bits = flags & 0x03;
  return sequence_bits == 0x01 || sequence_bits == 0x03;
}

bool is_connect_server_event(EventId event_id) {
  switch (event_id) {
    case EventId::kConnectionStarted:
    case EventId::kConnectionFailed:
    case EventId::kConnectionFinished:
      return true;
    default:
      return false;
  }
}

std::vector<uint8_t> Gunzip(const std::vector<uint8_t>& in) {
  static constexpr size_t kMaxDecompressedSize = 4 * 1024 * 1024;

  z_stream zs{};
  if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK) {
    throw std::runtime_error("inflateInit2 failed");
  }

  zs.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(in.data()));
  zs.avail_in = static_cast<unsigned int>(in.size());

  std::vector<uint8_t> out;
  out.resize(in.size() * 2 + 512);

  int ret = Z_OK;
  while (ret == Z_OK) {
    if (zs.total_out >= out.size()) {
      if (out.size() * 2 > kMaxDecompressedSize) {
        inflateEnd(&zs);
        throw std::runtime_error("gzip decompressed size exceeds limit");
      }
      out.resize(out.size() * 2);
    }
    zs.next_out = reinterpret_cast<Bytef*>(out.data() + zs.total_out);
    zs.avail_out = static_cast<unsigned int>(out.size() - zs.total_out);
    ret = inflate(&zs, Z_NO_FLUSH);
  }

  if (ret != Z_STREAM_END) {
    inflateEnd(&zs);
    throw std::runtime_error("gzip decode failed");
  }

  out.resize(zs.total_out);
  inflateEnd(&zs);
  return out;
}

}  // namespace

nlohmann::json Frame::JsonPayload() const {
  if (serialization != Serialization::kJson) {
    return nlohmann::json();
  }
  try {
    return nlohmann::json::parse(payload.begin(), payload.end());
  } catch (...) {
    spdlog::warn("json payload parse failed");
    return nlohmann::json();
  }
}

std::vector<uint8_t> BuildFrame(MessageType message_type, Serialization serialization,
                                EventId event_id, const std::vector<uint8_t>& payload,
                                const std::optional<std::string>& session_id,
                                Compression compression) {
  constexpr uint8_t kFlags = 0x4;
  std::vector<uint8_t> out;
  out.reserve(16 + payload.size());

  out.push_back(kHeaderByte0);
  out.push_back(static_cast<uint8_t>((static_cast<uint8_t>(message_type) << 4) | (kFlags & 0x0F)));
  out.push_back(static_cast<uint8_t>((static_cast<uint8_t>(serialization) << 4) |
                                     static_cast<uint8_t>(compression)));
  out.push_back(0x00);

  WriteU32(&out, static_cast<uint32_t>(event_id));

  if (!is_connect_class_event(event_id)) {
    if (!session_id.has_value() || session_id->empty()) {
      throw std::runtime_error("session_id required for session-level events");
    }
    WriteU32(&out, static_cast<uint32_t>(session_id->size()));
    out.insert(out.end(), session_id->begin(), session_id->end());
  }

  WriteU32(&out, static_cast<uint32_t>(payload.size()));
  out.insert(out.end(), payload.begin(), payload.end());
  return out;
}

std::vector<uint8_t> BuildJsonFrame(MessageType message_type, EventId event_id,
                                    const nlohmann::json& payload,
                                    const std::optional<std::string>& session_id) {
  const auto dumped = payload.dump();
  return BuildFrame(message_type, Serialization::kJson, event_id,
                    std::vector<uint8_t>(dumped.begin(), dumped.end()), session_id,
                    Compression::kNone);
}

std::vector<uint8_t> BuildAudioFrame(EventId event_id, const std::string& session_id,
                                     const std::vector<uint8_t>& audio_bytes) {
  return BuildFrame(MessageType::kAudioOnlyRequest, Serialization::kRaw, event_id, audio_bytes,
                    session_id, Compression::kNone);
}

Frame DecodeFrame(const std::vector<uint8_t>& data) {
  if (data.size() < 4) {
    throw std::runtime_error("frame too short");
  }

  const uint8_t header_size_words = data[0] & 0x0F;
  if (header_size_words == 0) {
    throw std::runtime_error("frame invalid header size");
  }

  size_t offset = static_cast<size_t>(header_size_words) * 4;
  if (offset > data.size()) {
    throw std::runtime_error("frame truncated header");
  }

  const uint8_t b1 = data[1];
  const uint8_t b2 = data[2];

  Frame frame;
  frame.message_type = static_cast<MessageType>((b1 >> 4) & 0x0F);
  frame.flags = b1 & 0x0F;
  frame.serialization = static_cast<Serialization>((b2 >> 4) & 0x0F);
  frame.compression = static_cast<Compression>(b2 & 0x0F);

  if (frame.message_type == MessageType::kErrorInfo) {
    frame.error_code = ReadU32(data, &offset);
  }

  if (HasSequence(frame.flags)) {
    frame.sequence = ReadI32(data, &offset);
  }

  if (HasEvent(frame.flags)) {
    frame.event_id = static_cast<EventId>(ReadU32(data, &offset));
  }

  if (frame.event_id.has_value()) {
    if (is_connect_server_event(*frame.event_id)) {
      // connect_id is optional in connect-class server events. Probe first and only consume
      // when the remaining layout can still hold payload_size + payload.
      size_t probe = offset;
      const uint32_t connect_id_size = ReadU32(data, &probe);
      const size_t remaining_after_size = data.size() - probe;
      if (remaining_after_size >= static_cast<size_t>(connect_id_size) + sizeof(uint32_t)) {
        offset = probe;
        if (connect_id_size > 0) {
          frame.connect_id =
              std::string(reinterpret_cast<const char*>(data.data() + offset), connect_id_size);
          offset += connect_id_size;
        }
      }
    } else if (!is_connect_class_event(*frame.event_id)) {
      const uint32_t sid_size = ReadU32(data, &offset);
      if (sid_size > 0) {
        if (offset + sid_size > data.size()) {
          throw std::runtime_error("frame missing session id");
        }
        frame.session_id = std::string(reinterpret_cast<const char*>(data.data() + offset), sid_size);
        offset += sid_size;
      }
    }
  }

  frame.payload_size = ReadU32(data, &offset);
  if (offset + frame.payload_size > data.size()) {
    throw std::runtime_error("frame missing payload bytes");
  }
  frame.payload.assign(data.begin() + static_cast<long>(offset),
                       data.begin() + static_cast<long>(offset + frame.payload_size));

  if (frame.compression == Compression::kGzip && !frame.payload.empty()) {
    frame.payload = Gunzip(frame.payload);
    frame.payload_size = static_cast<uint32_t>(frame.payload.size());
  }

  return frame;
}

}  // namespace xiaoai_plus::realtime::protocol
