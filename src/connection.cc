#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <memory>

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
    // message_.clear();
    int bytes_read = ReadData();
    if (bytes_read == -1) {
      // Message bigger than buffer size
      return false;
    } else if (bytes_read == 0) {
      // client triggered EPOLLIN but sent no data
      // (usually due to remote socket being closed)
      return false;
    }
    // Reset request obj
    req_ = std::make_unique<Request>(buffer_);
    // Reset response obj
    res_ = std::make_unique<Response>();
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
  if (bytes_read == buffer_size_) {return -1;}
  buffer_[bytes_read] = '\0';
  return bytes_read;
}


bool Connection::WriteReady() {

}

void Connection::SendResponse() {
  std::string response = res_->ConstructResponse().c_str();
  write(fd_, response.c_str(), response.size());
}


