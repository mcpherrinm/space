#pragma once
//  Low-level network primitives
#include <string.h>
#include <stdint.h>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <queue>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

class Address {
private:
  union {
    char v4[INET_ADDRSTRLEN];
    char v6[INET6_ADDRSTRLEN];
  } namebuf;
public:  // but not portable
  union {
    struct sockaddr sa;
    struct sockaddr_in sa_in;
    struct sockaddr_storage sa_storage;
  };
public:
  Address(const char *, uint16_t port = 1255);
  Address(uint32_t, uint16_t port = 1255);
  Address(){};
  const char *ip_str();
};

enum class MessageType : uint16_t {
  INVALID = 0,
  JOIN_GAME, ADD_STATION, LEAVE_GAME, REMOVE_STATION,
  ACK = 0xFFFF
};

// Header on network messages
struct MessageHeader {
  MessageType typetag;
  uint16_t messageid;
};

// This is what lives in the various queues.
// The header is followed by the message
// so don't copy it off!
struct Message {
  std::chrono::time_point<std::chrono::steady_clock> time_in_Q;
  int resends;

  MessageHeader* header;
  ssize_t data_len; // includes size of header
};

class Connection {
  Address server;
  int sock;
  int nextmessage = 0;
  Connection(const Address &server);

  std::mutex send_Q_mutex;
  std::condition_variable send_Q_cv;
  std::queue<Message> send_Q;

  std::mutex ack_Q_mutex;
  std::vector<Message> ack_Q;

  void send_loop();
  void recv_loop();

public:
  static void run(const Address &server);

  bool send(const Message&);
};
