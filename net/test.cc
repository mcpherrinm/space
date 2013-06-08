// test brogram

#include "net.hh"

int main(int argc, char **argv) {
  Address testserv(argc > 1 ? argv[1] : "127.0.0.1");
  Connection c(testserv);

  Message *joinmsg = new Message;
  joinmsg->typetag = MessageType::JOIN_GAME;
  c.send(joinmsg, sizeof joinmsg);

  Message *two = new Message;
  two->typetag = MessageType::JOIN_GAME;
  c.send(two, sizeof two);

  union {
    char buf[256];
    Message m;
  };
  scanf("%s", buf+sizeof(m));
  m.typetag = MessageType::DEBUG;
  c.send(&m, 256);

  char buffa[256];
  scanf("%s", buffa);
}
