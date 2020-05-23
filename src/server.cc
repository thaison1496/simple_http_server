#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <stdexcept>
#include <vector>

#include "server.hh"
#include "connection.hh"
#include "utils.hh"

Server::Server(const ServerConfig& cfg) : 
    cfg_(cfg),
    listen_fd_(-1),
    epoll_fd_(-1) {
  buffer_ = new char[cfg.buffer_size];
  logger_ = CreateLogger(cfg.logger);

  sockaddr_in sin = {0};

  sin.sin_addr.s_addr = inet_addr(cfg.addr.c_str());
  sin.sin_family = AF_INET;
  sin.sin_port = htons(cfg.port);

  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ <= 0)
    throw std::runtime_error("socket() failed, error code: " + std::to_string(errno));

  if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)))
    throw std::runtime_error("bind() failed, error code: " + std::to_string(errno));

  if (!SetNonblocking(listen_fd_))
    throw std::runtime_error("SetNonBlocking() failed, error code: " + std::to_string(errno));

  if (listen(listen_fd_, SOMAXCONN) == -1)
    throw std::runtime_error("listen() failed, error code: " + std::to_string(errno));

  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1)
    throw std::runtime_error("epoll_create1() failed, error code: " + std::to_string(errno));

  epoll_event e_event;
  e_event.events = EPOLLIN;
  e_event.data.fd = listen_fd_;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &e_event) == -1)
    throw std::runtime_error("epoll_ctl() failed, error code: " + std::to_string(errno));

  events_ = new epoll_event[1024];
}


void Server::Route(
    vector<string> allowed_methods,
    string uri,
    Handler handler) {
  Router route{allowed_methods, uri, handler};
  routes_.emplace_back(route);
}


void Server::Listen() {
  while (true) {
    int num_fds = epoll_wait(epoll_fd_, events_, 64, 1000);
    if (num_fds == -1) {continue;}

    // iterate signaled fds
    for (int i = 0; i < num_fds; i++) {
      // notifications on listening fd are incoming client connections
      if (events_[i].data.fd == listen_fd_) {
        AcceptConnection();
      } else {
        ProcessEvent(events_[i]);
      }
    }
    
    // Clear timed-out connections
    // Estimation, only call get time once
    uint32_t time_now = time(nullptr);
    for (auto itr = conn_map_.begin(); itr != conn_map_.end();) {
      auto& conn = itr->second;
      if (Timeout(conn, time_now)) {
        logger_->Log("[+] server close fd: %d\n", conn->fd_);
        close(conn->fd_);
        delete conn;
        itr = conn_map_.erase(itr);
      } else {
        ++itr;
      }
    }
  }
}


void Server::AcceptConnection() {
  sockaddr_in client_sin;
  socklen_t sin_size = sizeof(client_sin);

  int client_fd = accept(listen_fd_, 
      reinterpret_cast<sockaddr*>(&client_sin), &sin_size);
  if (client_fd == -1) {
    logger_->Log("accept() failed, error code: %d\n", errno);
    return;
  }

  if (!SetNonblocking(client_fd)) {
    logger_->Log("failed to put fd into non-blocking mode, error code: %d\n", errno);
    return;
  }

  epoll_event ev;
  //client events will be handled in edge-triggered mode
  ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;

  // initialize new connection
  auto conn = new Connection(client_fd, buffer_, cfg_.buffer_size);
  conn_map_[client_fd] = conn;
  ev.data.ptr = conn;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) == 1) {
    logger_->Log("epoll_ctl() failed, error code: %d\n", errno);
    CloseConnection(client_fd);
    return;
  }

  logger_->Log("[+] new client: %s:%d %d\n",
      inet_ntoa(client_sin.sin_addr),
      ntohs(client_sin.sin_port),
      client_fd);
}


void Server::ProcessEvent(const epoll_event& ev) {
  auto conn = reinterpret_cast<Connection*>(ev.data.ptr);
  if (!conn->HandleEvent(ev)) {
    logger_->Log("[+] client close fd: %d\n", conn->fd_);
    CloseConnection(conn->fd_);
  }
}


void Server::CloseConnection(int client_fd) {
  auto itr = conn_map_.find(client_fd);
  if (itr == conn_map_.end()) {return;}
  delete itr->second;
  conn_map_.erase(itr);
  close(client_fd);
}

inline bool Server::Timeout(Connection* conn, uint32_t time_now) {
  return conn->last_active_ + cfg_.timeout_secs < time_now;
}

Server::~Server() {
  delete[] events_;
  delete[] buffer_;
}
