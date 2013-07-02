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

#include "message.hh"

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

// This is what lives in the various queues.
struct QueuedMessage {
  std::chrono::time_point<std::chrono::steady_clock> time_in_Q;
  int resends;

  Message* data;
  ssize_t data_len; // includes size of header
};

template <class CompletionPolicy, class DispatchPolicy>
class Connection: public CompletionPolicy, public DispatchPolicy {
  Address server;
  int sock;
  int nextmessage = 0;

  std::mutex send_Q_mutex;
  std::condition_variable send_Q_cv;
  std::queue<QueuedMessage> send_Q;

  std::mutex ack_Q_mutex;
  std::list<QueuedMessage> ack_Q;
  std::thread send_thread;
  std::thread recv_thread;

public:
  Connection(const Address &server_): server(server_) {
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    send_thread = std::thread(&Connection<CompletionPolicy,DispatchPolicy>::send_loop, this);
    recv_thread = std::thread(&Connection<CompletionPolicy,DispatchPolicy>::recv_loop, this);
    send_thread.detach();
    recv_thread.detach();
  }

private:
  bool ack_msg(int id) {
    std::lock_guard<std::mutex> lk(ack_Q_mutex);
    for(auto i = ack_Q.begin(); i != ack_Q.end(); i++) {
      if(i->data->messageid == id) {
        //disposeMessage(i->data);
        ack_Q.erase(i);
        return true;
      }
    }
    return false;
  }

  // On the new thread, do a select() loop
  void recv_loop() {
    while(1) {
      union {
        char recvbuf[1500];
        Message incoming;
      };
      Address from;
      socklen_t fromlen = sizeof from.sa;
      ssize_t recvsize = recvfrom(sock,
                                  recvbuf, sizeof recvbuf,
                                  0,
                                  (struct sockaddr*)&from.sa, &fromlen);

      // TODO sort out dispatching
      if(incoming.typetag == MessageType::ACK) {
        ack_msg(incoming.messageid);
      } else {
        DispatchPolicy::dispatch(&incoming, recvsize, &from);
      }
    }
  }

  // This is pretty gross/bad.
  void send_loop() {
    while(1) {
      bool send = false;
      QueuedMessage m, ackm;
      {
        std::unique_lock<std::mutex> lk(send_Q_mutex);
        if(send_Q.empty()) {
          if(ack_Q.empty()) {
            send_Q_cv.wait(lk);
          } else {
            // Maybe, more appropriately, wait until the next ack timeout.
            send_Q_cv.wait_for(lk, std::chrono::milliseconds(31));
          }
        }
        if(!send_Q.empty()) {
          m = send_Q.front();
          send_Q.pop();
          send = true;
        }
      }
      bool resend = false;
      {
        std::lock_guard<std::mutex> lk(ack_Q_mutex);
        if(send) {
          m.time_in_Q = std::chrono::steady_clock::now();
          ack_Q.push_back(m);
        }

        // check if any unacked packets need to be resent.
        if(!ack_Q.empty()) {
          ackm = ack_Q.front();
          auto now = std::chrono::steady_clock::now();
          auto count = std::chrono::duration_cast<std::chrono::milliseconds>(now - ackm.time_in_Q).count();
          if(count > 30) {
            ack_Q.erase(ack_Q.begin());
            resend = true;
          }
        }
      }
      if(resend) {
        ackm.resends += 1;
        if(ackm.resends > 10) {
          printf("Stopping resending at 10 -- need to handle this\n");
          return;
        }
        std::lock_guard<std::mutex> lk(send_Q_mutex);
        send_Q.push(ackm);
      }
      if(send) {
        if(m.data->typetag == MessageType::INVALID) {
          printf("Tried to send invalid message\n");
          return;
        }
        ssize_t sent = sendto(sock,
                              m.data,
                              m.data_len,
                              0, /*flags*/
                              (struct sockaddr*)&server.sa,
                              sizeof server.sa);
        if(sent < 0) {
          perror("sendto");
          return;
        }
      }
    }
  }

public:
  bool send(Message *msg, ssize_t datalen) {
    std::lock_guard<std::mutex> lk(send_Q_mutex);
    QueuedMessage m;
    m.data = msg;
    m.data_len = datalen;
    msg->messageid = ++nextmessage;
    send_Q.push(m);
    send_Q_cv.notify_one();
    return true;  // If I switch to a bounded Queue, return false on full
                  // or on dead connection
  }
};
