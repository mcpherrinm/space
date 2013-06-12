#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h> /* for close() for socket */ 
#include <stdlib.h>
#include <arpa/inet.h>
#include "net.hh"

int main(void) {
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  struct sockaddr_in sa;

  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(1255);

  if (-1 == bind(sock,(struct sockaddr *)&sa, sizeof(sa))) {
    perror("error bind failed");
    close(sock);
    exit(EXIT_FAILURE);
  }

  for (;;) {
    union {
      char buffer[2046];
      Message message;
    };
    ssize_t recsize;
    Address from;
    socklen_t fromlen = sizeof(from.sa_storage);
    recsize = recvfrom(sock,
                       (void *)buffer, sizeof(buffer),
                       0,
                       &from.sa, &fromlen);

    if(recsize < sizeof(Message)) {
      printf("Got too small message, skipping\n");
      break;
    }
    printf("got message of size %zd from %s\n"
           "type: %d, id: %d\n",
           recsize, from.ip_str(),
           (int)message.typetag, message.messageid);

    switch(message.typetag) {
     case MessageType::DEBUG:
      printf("DEBUG: %s\n", ((char*)&message) + sizeof(Message));
      break;
     case MessageType::JOIN_GAME:
      printf("Join game by %s\n", ((JoinMessage*)&message)->username);
      break;
     case MessageType::INVALID:
      printf("WHOA, BRO. Invalid!\n");
     default:
      break;
    }
    message.typetag = MessageType::ACK; // MessageID number stays the same
    sendto(sock, &message, sizeof( Message ), 0, &from.sa, fromlen);
  }
}
