// test brogram

#include "net.hh"

int main(int argc, char **argv) {
  Address testserv(argc > 1 ? argv[1] : "127.0.0.1");
  Connection c(testserv);
  Message joinmsg;
  joinmsg.data_len = sizeof(MessageHeader);
  joinmsg.header = (MessageHeader*)malloc(joinmsg.data_len);
  joinmsg.header->typetag = MessageType::JOIN_GAME;
  joinmsg.resends = 0;
  c.send(joinmsg);

  Message two;
  two.data_len = sizeof(MessageHeader);
  two.header = (MessageHeader*)malloc(two.data_len);
  two.header->typetag = MessageType::JOIN_GAME;
  two.resends = 0;
  c.send(two);
  c.finish();
}
