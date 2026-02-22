#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "realtime/types.hpp"

namespace xiaoai_plus::realtime::protocol {

struct Frame {
  MessageType message_type{MessageType::kFullServerResponse};
  uint8_t flags{0};
  Serialization serialization{Serialization::kRaw};
  Compression compression{Compression::kNone};
  std::optional<EventId> event_id;
  std::optional<int32_t> sequence;
  std::optional<std::string> session_id;
  std::optional<std::string> connect_id;
  uint32_t payload_size{0};
  std::vector<uint8_t> payload;
  std::optional<uint32_t> error_code;

  nlohmann::json JsonPayload() const;
};

std::vector<uint8_t> BuildFrame(MessageType message_type, Serialization serialization,
                                EventId event_id, const std::vector<uint8_t>& payload,
                                const std::optional<std::string>& session_id,
                                Compression compression);

std::vector<uint8_t> BuildJsonFrame(MessageType message_type, EventId event_id,
                                    const nlohmann::json& payload,
                                    const std::optional<std::string>& session_id);

std::vector<uint8_t> BuildAudioFrame(EventId event_id, const std::string& session_id,
                                     const std::vector<uint8_t>& audio_bytes);

Frame DecodeFrame(const std::vector<uint8_t>& data);

}  // namespace xiaoai_plus::realtime::protocol
