CFLAGS=-Wall -Wextra -std=c++11 -stdlib=libc++
WERROR=-Wno-error=unused -Werror

test: test.cc unix.o
	clang++ test.cc -lpthread ${CFLAGS} unix.o -o test
unix.o: unix.cc net.hh
	clang++ -c ${CFLAGS} -o unix.o unix.cc

udp-server: udp-server.cc net.hh
	clang++ ${CFLAGS} -o udp-server udp-server.cc

listtest: listtest.cc
	clang++ ${CFLAGS} -o listtest listtest.cc
