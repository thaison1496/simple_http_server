// #pragma once
#ifndef CPPSERVER_HTTP_HTTP_REQUEST_H
#define CPPSERVER_HTTP_HTTP_REQUEST_H

#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>

#include "request.hh"
#include "response.hh"

using std::vector;
using std::string;
using Handler = std::function<void(const Request&, Response&)>;

struct ServerConfig {
  string addr = "0.0.0.0";
  uint16_t port = 1337;
  uint32_t num_threads = 1;
  size_t buffer_size = 8096;
  bool enable_log = true;
  uint32_t timeout_secs = 5;
  string logger = "";
};


struct Router {
  vector<string> allowed_methods;
  string uri;
  Handler handler;
};


class Logger {
public:
  virtual void Log(const char *, ...) = 0;
};


class NullLogger : public Logger {
public:
  virtual void Log(const char *, ...) {}
};


class StdoutLogger : public Logger {
public:
  virtual void Log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
  }
};


class FileLogger : public Logger {
public:
  FileLogger(string file_name) {   
    p_file = fopen(file_name.c_str(), "w");
  }

  virtual void Log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(p_file, fmt, args);
    va_end(args);
  }

private:
  FILE* p_file;
};


inline std::unique_ptr<Logger> CreateLogger(string logger_type) {
  if (logger_type.empty()) {
    return std::unique_ptr<Logger>(new NullLogger());
  } else if (logger_type == "stdout") {
    return std::unique_ptr<Logger>(new StdoutLogger());
  }
  return std::unique_ptr<Logger>(new FileLogger(logger_type));
}


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