#include <stdio.h>

#include "message.hh"

#include <cstddef>

int main() {
  Message m;
  printf("Size of Message is %ld\n", sizeof(m));
  printf("Offset of Message tag is %ld\n", offsetof(Message, typetag));
  printf("Offset of Message id is %ld\n", offsetof(Message, messageid));

  ClientMessage m2;
  printf("Size of ClientMessage is %ld\n", sizeof(m2));
  printf("Offset of ClientMessage header is %ld\n", offsetof(ClientMessage, m));
  printf("Offset of ClientMessage d is %ld\n", offsetof(ClientMessage, d));

  printf("addr of m2 %p and m2.m %p\n", &m2, &m2.m);
}
