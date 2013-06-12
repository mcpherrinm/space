// test brogram

#include "net.hh"

int main(int argc, char **argv) {
  Address testserv(argc > 1 ? argv[1] : "127.0.0.1");
  Connection c(testserv);

  JoinMessage *join = new JoinMessage("John");
  c.send((Message*)join, sizeof join);

  union {
    char buf[256];
    Message m;
  };
  // Hawkward:  That buffer m could become
  // invalid if the network is slow or whatever
  // and this function returns.

  // In general, since "send" is async, I need to be careful.
  // But I don't have a way of telling if a message has been sent,
  // so managing buffers is grosslike.

  // I could mandate Message be a new'd object for later delete'ion
  // on successful send, or otherwise come from a specified pool.  Given
  // convenient ways of using those, it might not be bad.  
  scanf("%s", buf+sizeof(m));
  m.typetag = MessageType::DEBUG;
  c.send(&m, 256);

  std::this_thread::sleep_for(std::chrono::hours(999));
}
