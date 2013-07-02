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

