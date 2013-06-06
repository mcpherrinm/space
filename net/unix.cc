#include "net.hh"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Address::Address(const char *ip, uint16_t port) {
  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(ip);
  sa.sin_port = htons(port);
}

// Todo: figure out what the right handle to return
// from this function is.
void Connection::run(const Address &server) {
  auto conn = new Connection(server);
  std::thread send_thread(&Connection::send_loop, conn);
  std::thread recv_thread(&Connection::recv_loop, conn);
  send_thread.join(); // TODO, return these instead
  recv_thread.join();
}

Connection::Connection(const Address &server_): server(server_) {
  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

  Message joinmsg;
  joinmsg.data_len = sizeof(MessageHeader);
  joinmsg.header = (MessageHeader*)malloc(joinmsg.data_len);
  joinmsg.header->typetag = MessageType::JOIN_GAME;
  joinmsg.header->messageid = 123;
  joinmsg.resends = 0;
  send_Q.push(joinmsg);
}


// On the new thread, do a select() loop
void Connection::recv_loop() {
  while(1) {
    union {
      char recvbuf[1500];
      MessageHeader mh;
    };
    Address from;
    socklen_t fromlen = sizeof from.sa;
    ssize_t recvsize = recvfrom(sock,
                                recvbuf, sizeof recvbuf,
                                0,
                                (struct sockaddr*)&from.sa, &fromlen);

    if(mh.typetag == MessageType::ACK) {
      {
        std::lock_guard<std::mutex> lk(ack_Q_mutex);
        for(auto i = ack_Q.begin(); i != ack_Q.end(); i++) {
          printf("Ack Q m has id %d\n", (*i).header->messageid);
          if((*i).header->messageid == mh.messageid) {
            printf("acking existing packet!\n");
            //ack_Q.erase(i);
            //ack_Q.erase(ack_Q.begin());
            break;
          }
        }
      }
    } else {
      printf("Got a message of size %ld from %s of type %d\n",
        recvsize, from.ip_str(), (int)mh.typetag);
    }
  }
}

void Connection::send_loop() {
  while(1) {
    bool send = false;
    Message m, ackm;
    {
      std::unique_lock<std::mutex> lk(send_Q_mutex);
      if(send_Q.empty()) {
        if(ack_Q.empty()) {
          send_Q_cv.wait(lk);
        } else {
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
      ackm = ack_Q.front();
      auto now = std::chrono::steady_clock::now();
      auto count = std::chrono::duration_cast<std::chrono::milliseconds>(now - ackm.time_in_Q).count();
      if(count > 30) {
        //ack_Q.erase(ack_Q.begin());
        resend = true;
      }
    }
    if(resend) {
      ackm.resends += 1;
      if(ackm.resends > 10) {
        return;
      }
      std::lock_guard<std::mutex> lk(send_Q_mutex);
      send_Q.push(ackm);
    }
    if(send) {
      ssize_t sent = sendto(sock,
                            m.header,
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

bool Connection::send(const Message& msg) {
  std::lock_guard<std::mutex> lk(send_Q_mutex);
  send_Q.push(msg);
  send_Q_cv.notify_one();
  return true;  // If I switch to a bounded Queue, return false on full
                // or on dead connection
}
