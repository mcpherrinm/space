CFLAGS=-Wall -Wextra -std=c++11
WERROR=-Wno-error=unused -Werror

CXX=g++-4.8

test: test.cc unix.o
	${CXX} test.cc -lpthread ${CFLAGS} unix.o -o test
unix.o: unix.cc net.hh
	${CXX} -c ${CFLAGS} -o unix.o unix.cc

udp-server: udp-server.cc net.hh unix.o
	${CXX} ${CFLAGS} -o udp-server udp-server.cc unix.o

struct: struct.cc
	${CXX} ${CFLAGS} -o struct struct.cc
