// #pragma once
#ifndef CPPSERVER_HTTP_HTTP_REQUEST_H
#define CPPSERVER_HTTP_HTTP_REQUEST_H

#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <functional>

#include "request.hh"
#include "response.hh"

using std::vector;
using std::string;
using Handler = std::function<void(const Request&, Response&)>;

struct ServerConfig {
  const char* addr = "0.0.0.0";
  uint16_t port = 1337;
  uint32_t num_threads = 1;
  size_t buffer_size = 8096;
  bool enable_log = true;
  uint32_t timeout_secs_ = 5;
};


struct Router {
  vector<string> allowed_methods;
  string uri;
  Handler handler;
};


inline bool SetNonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if(flags == -1)
    return false;

  flags |= O_NONBLOCK;

  if(fcntl(fd, F_SETFL, flags) == -1)
    return false;

  return true;
}

#endif // CPPSERVER_HTTP_HTTP_REQUEST_H