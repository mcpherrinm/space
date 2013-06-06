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
  struct sockaddr_storage sa_from;
  union {
    char buffer[1024];
    MessageHeader message;
  };
  ssize_t recsize;
  socklen_t fromlen;

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
    recsize = recvfrom(sock,
                       (void *)buffer, sizeof(buffer),
                       0,
                       (struct sockaddr *)&sa_from, &fromlen);
    printf("got message of size %zd from %s\n"
           "type: %d, id: %d\n",
           recsize, inet_ntoa(((struct sockaddr_in*)&sa_from)->sin_addr),
           (int)message.typetag, message.messageid);
    message.typetag = MessageType::ACK; // MessageID number stays the same
    sendto(sock, &message, sizeof( MessageHeader), 0, (struct sockaddr *)&sa_from, fromlen);
    sendto(sock, &message, sizeof( MessageHeader), 0, (struct sockaddr *)&sa_from, fromlen);
    sendto(sock, &message, sizeof( MessageHeader), 0, (struct sockaddr *)&sa_from, fromlen);
    sendto(sock, &message, sizeof( MessageHeader), 0, (struct sockaddr *)&sa_from, fromlen);
  }
}
