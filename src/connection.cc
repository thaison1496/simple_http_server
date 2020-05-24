#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

#include "connection.hh"

Connection::Connection(int fd, char* buffer, const int buffer_size) :
    fd_(fd),
    buffer_(buffer),
    buffer_size_(buffer_size),
    last_active_(time(nullptr)) {}


// Return false if the connection should be closed
bool Connection::HandleEvent(const epoll_event& ev) {
  last_active_ = time(nullptr);

  // we got some data from the client
  if (ev.events & EPOLLIN) {
    message_.clear();
    ReadData();
    // client triggered EPOLLIN but sent no data
    // (usually due to remote socket being closed)
    if (message_.empty()) {
      return false;
    }
    SendResponse();
    return true;
  }

  // the client closed the connection
  // (should be after EPOLLIN as client can send data then close)
  if(ev.events & EPOLLRDHUP) {
    return false;
  }

  // fd is ready to be written
  // if (ev.events & EPOLLOUT) {
  //   SendResponse();
  //   return true;
  // }
  return false;
}


void Connection::ReadData() {
  int bytes_read;
  // must drain the entire read buffer as we won't
  // get another event until client sends more data
  while (true) {
    bytes_read = recv(fd_, buffer_, static_cast<size_t>(buffer_size_), 0);
    if (bytes_read <= 0)
      break;
    message_.append(buffer_, static_cast<size_t>(bytes_read));
  }
}


void Connection::SendResponse() {
  // just echo back
  // write(fd_, message_.c_str(), message_.size());
  res_ = new Response();
  std::string response = res_->ConstructResponse().c_str();
  write(fd_, response.c_str(), response.size());
}


