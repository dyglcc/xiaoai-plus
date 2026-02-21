#pragma once

#include <cstdint>

namespace xiaoai_plus::realtime::protocol {

enum class MessageType : uint8_t {
  kFullClientRequest = 0x1,
  kAudioOnlyRequest = 0x2,
  kFullServerResponse = 0x9,
  kAudioOnlyResponse = 0xB,
  kErrorInfo = 0xF,
};

enum class Serialization : uint8_t {
  kRaw = 0x0,
  kJson = 0x1,
};

enum class Compression : uint8_t {
  kNone = 0x0,
  kGzip = 0x1,
};

enum class EventId : uint32_t {
  kStartConnection = 1,
  kFinishConnection = 2,

  kConnectionStarted = 50,
  kConnectionFinished = 52,

  kStartSession = 100,
  kFinishSession = 102,

  kSessionStarted = 150,
  kSessionFinished = 152,
  kSessionFailed = 153,

  kTaskRequest = 200,
  kSayHello = 300,

  kTtsResponse = 352,
  kTtsEnded = 359,

  kAsrInfo = 450,
  kAsrResponse = 451,

  kChatResponse = 550,
  kChatEnded = 559,

  kDialogCommonError = 599,
};

inline constexpr uint8_t kProtocolVersion = 0x1;
inline constexpr uint8_t kHeaderSize = 0x1;
inline constexpr uint8_t kHeaderByte0 = static_cast<uint8_t>((kProtocolVersion << 4) | kHeaderSize);

inline bool is_connect_class_event(EventId event_id) {
  switch (event_id) {
    case EventId::kStartConnection:
    case EventId::kFinishConnection:
    case EventId::kConnectionStarted:
    case EventId::kConnectionFinished:
      return true;
    default:
      return false;
  }
}

}  // namespace xiaoai_plus::realtime::protocol
