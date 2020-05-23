#pragma once

#include <unistd.h>
#include <sys/epoll.h>

#include "request.hh"
#include "response.hh"

// Handle a TCP connection to a client (read, write, close, timeout)
class Connection {
public:
  Connection(int fd, char* buffer, const int buffer_size);

  bool HandleEvent(const epoll_event& ev);

  void ReadData();

  void SendResponse();
  
  int fd_;
  char* buffer_;
  const int buffer_size_;
  uint32_t last_active_;
  std::string message_;
  Request* req_;
  Response* res_;
};