#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <memory>
#include <algorithm>

#include "connection.hh"
#include "http_parser.hh"
#include "defaults.hh"

Connection::Connection(int fd, char* buffer, const int buffer_size, const Routes& route) :
    fd_(fd),
    buffer_(buffer),
    buffer_size_(buffer_size),
    last_active_(time(nullptr)),
    route_(route) {}


// Return false if the connection should be closed#include <iostream>
// using namespace std;
// TODO: implement keep alive
bool Connection::HandleEvent(const epoll_event& ev) {
  last_active_ = time(nullptr);

  // we got some data from the client
  if (ev.events & EPOLLIN) {
    int bytes_read = ReadData();
    if (bytes_read == -1) {
      // Message bigger than buffer size
      res_ = defaults::response_413;
      SendResponse();
      return false;
    } else if (bytes_read == 0) {
      // client triggered EPOLLIN but sent no data
      // (usually due to remote socket being closed)
      return false;
    }

    req_ = http_parser::ParseHttpRequest(buffer_, bytes_read);
    if (req_.valid) {  
      // Find appropriate handler to process this request
      FindHandler();
      if (handler_ != nullptr) {
        res_.return_code = 200;
        handler_(req_, res_);
      }
    } else {
      res_ = defaults::response_400;
    }

    SendResponse();
    return false;
  }

  // the client closed the connection
  // (should be after EPOLLIN as client can send data then close)
  if(ev.events & EPOLLRDHUP) {
    return false;
  }

  return false;
}


int Connection::ReadData() {
  int bytes_read;
  // must drain the entire read buffer as we won't
  // get another event until client sends more data
  bytes_read = recv(fd_, buffer_, static_cast<size_t>(buffer_size_), 0);
  if (bytes_read == buffer_size_) {
    // flush remaining data
    while (bytes_read > 0) {
      bytes_read = recv(fd_, buffer_, static_cast<size_t>(buffer_size_), 0);
    }
    return -1;
  }
  buffer_[bytes_read] = '\0';
  return bytes_read;
}


void Connection::FindHandler() {
  res_ = defaults::response_404;
  if (route_.empty()) return;
  for (auto& route : route_) {
    // TODO: handle regex patterns
    if (req_.uri == route.pattern) {
      if (std::find(route.allowed_methods.begin(), 
          route.allowed_methods.end(), 
          req_.method) != route.allowed_methods.end()) {
        handler_ = route.handler;
      } else {
        res_ = defaults::response_405;
      }
      return;
    }
  }
}


void Connection::SendResponse() {
  std::string response = http_parser::ConstructHttpResponse(res_);
  write(fd_, response.c_str(), response.size());
}


