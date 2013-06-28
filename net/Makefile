CXXFLAGS=-Wall -Wextra -std=c++11
WERROR=-Wno-error=unused -Werror

CXX=g++-4.8
CXX=g++

test: test.cc unix.o message.hh
	${CXX} test.cc -lpthread ${CXXFLAGS} unix.o -o test
unix.o: unix.cc net.hh message.hh
	${CXX} -c ${CXXFLAGS} -o unix.o unix.cc

udp-server: udp-server.cc net.hh unix.o message.hh
	${CXX} ${CXXFLAGS} -o udp-server udp-server.cc unix.o

struct: struct.cc
	${CXX} ${CXXFLAGS} -o struct struct.cc
