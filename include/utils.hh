#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <chrono>

using std::vector;
using std::string;

struct ServerConfig {
  string addr = "0.0.0.0";
  uint16_t port = 1337;
  size_t num_threads = 1;
  size_t buffer_size = 8096;
  bool enable_log = true;
  uint32_t timeout_secs = 5;
  string logger = "";
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
    time_begin = std::chrono::steady_clock::now();
  }

  virtual void Log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(p_file, "[%ld] ", std::chrono::duration_cast<std::chrono::microseconds>
        (std::chrono::steady_clock::now() - time_begin).count());
    vfprintf(p_file, fmt, args);
    va_end(args);
  }

private:
  FILE* p_file;
  std::chrono::steady_clock::time_point time_begin;
};


inline std::shared_ptr<Logger> CreateLogger(string logger_type) {
  if (logger_type.empty()) {
    return std::shared_ptr<Logger>(new NullLogger());
  } else if (logger_type == "stdout") {
    return std::shared_ptr<Logger>(new StdoutLogger());
  }
  return std::shared_ptr<Logger>(new FileLogger(logger_type));
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
