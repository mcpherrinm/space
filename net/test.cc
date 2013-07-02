// test brogram

#include "net.hh"

class DeleteCompletion {
  void dispose(Message *m){delete m;};
};

class Dispatcher {
  public:
  void dispatch(Message *m, ssize_t len, Address *from) {
    printf("Got a message of size %ld from %s of type %d\n",
      len, from->ip_str(), (int)m->typetag);
  }
};

int main(int argc, char **argv) {
  Address testserv(argc > 1 ? argv[1] : "127.0.0.1");
  Connection<DeleteCompletion,Dispatcher> c(testserv);

  JoinMessage *join = new JoinMessage("John");
  c.send((Message*)join, sizeof(JoinMessage));

  union {
    char buf[256];
    Message m;
  };
  scanf("%s", buf+sizeof(m));
  m.typetag = MessageType::DEBUG;
  c.send(&m, 256);

  std::this_thread::sleep_for(std::chrono::hours(999));
}
