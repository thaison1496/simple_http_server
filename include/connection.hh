#pragma once

#include <unistd.h>
#include <sys/epoll.h>

#include "data_types.hh"

// Handle a TCP connection to a client (read, write, close, timeout)
class Connection {
public:
  Connection(int fd, char* buffer, const int buffer_size, const Routes& route);

  bool HandleEvent(const epoll_event& ev);

  int ReadData();

  void FindHandler();

  void SendResponse();
  
  int fd_;
  char* buffer_;
  const int buffer_size_;
  const Routes& route_;
  uint32_t last_active_;
  std::string message_;
  Request req_;
  Response res_;
  Handler handler_;
};