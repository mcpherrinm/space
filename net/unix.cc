#include "net.hh"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Address::Address(const char *ip, uint16_t port) {
  memset(&sa, 0, sizeof sa);
  sa_in.sin_family = AF_INET;
  sa_in.sin_addr.s_addr = inet_addr(ip);
  sa_in.sin_port = htons(port);
}
const char *Address::ip_str() {
  switch(sa.sa_family) {
      case AF_INET:
          inet_ntop(AF_INET, &(((struct sockaddr_in *)&sa)->sin_addr),  // but not portable
                  namebuf.v4, sizeof namebuf);
          break;

      case AF_INET6:
          inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)&sa)->sin6_addr),
                  namebuf.v6, sizeof namebuf);
          break;

      default:
          sprintf(namebuf.v6, "Unknown AF %d", sa.sa_family);
          return namebuf.v4;
  }

  return namebuf.v4;
}

Connection::Connection(const Address &server_): server(server_) {
  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);


  send_thread = std::thread(&Connection::send_loop, this);
  recv_thread = std::thread(&Connection::recv_loop, this);
  send_thread.detach();
  recv_thread.detach();
}

bool Connection::ack_msg(int id) {
  std::lock_guard<std::mutex> lk(ack_Q_mutex);
  for(auto i = ack_Q.begin(); i != ack_Q.end(); i++) {
    if((*i).data->messageid == id) {
      ack_Q.erase(i);
      return true;
    }
  }
  return false;
}

// On the new thread, do a select() loop
void Connection::recv_loop() {
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
      printf("Got a message of size %ld from %s of type %d\n",
        recvsize, from.ip_str(), (int)incoming.typetag);
    }
  }
}

// This is pretty gross/bad.
void Connection::send_loop() {
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
      // free when acked!
      //free(m.header); // XXX Use a message pool instead?
      if(sent < 0) {
        perror("sendto");
        return;
      }
    }
  }
}

bool Connection::send(Message *msg, ssize_t datalen) {
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
