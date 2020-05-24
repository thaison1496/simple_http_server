#include "server_thread.hh"
#include <stdio.h>

void ServerThread::operator() () {
  printf("============ \n");
  while (true) {
    printf("1 \n");
  }
}