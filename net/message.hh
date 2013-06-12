#pragma once
#include <stdint.h>

enum class MessageType : uint16_t {
  /* Never sent: */
  INVALID = 0,
  /* Sent by client: */
  JOIN_GAME, ADD_STATION, LEAVE_GAME, REMOVE_STATION,
  /* Sent by server: */
  DATA_UPDATE,
  /*Sent by either: */
  DEBUG, ACK = 0xFFFF
};

// Header on network messages
struct Message {
  MessageType typetag;
  uint16_t messageid;
  Message():typetag(MessageType::INVALID) {};
  Message(MessageType t):typetag(t) {};
};

struct JoinMessage {
  Message header;
  char username[32];
  JoinMessage(const char *un): header(MessageType::JOIN_GAME) {
    strncpy(username, un, sizeof username);
  };
};

struct AddStation {
  Message header;
  int id;
};
