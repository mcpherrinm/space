#pragma once
#include <stdint.h>

enum class MessageType : uint16_t {
  INVALID = 0,
  JOIN_GAME, ADD_STATION, LEAVE_GAME, REMOVE_STATION,
  DEBUG,
  ACK = 0xFFFF
};

// Header on network messages
struct Message {
  MessageType typetag;
  uint16_t messageid;
};

struct JoinMessage {
  Message header;
};

