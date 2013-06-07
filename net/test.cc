// test brogram

#include "net.hh"

int main(int argc, char **argv) {
  Address testserv(argc > 1 ? argv[1] : "127.0.0.1");
  Connection c(testserv);
}
