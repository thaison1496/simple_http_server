#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <chrono>
#include <stdexcept>
#include <vector>

#include "connection.hh"
#include "server.hh"
#include "utils.hh"

Server::Server(const ServerConfig& cfg)
    : cfg_(cfg), listen_fd_(-1), epoll_fd_(-1) {
  logger_ = CreateLogger(cfg.logger);

  sockaddr_in sin = {0};

  sin.sin_addr.s_addr = inet_addr(cfg.addr.c_str());
  sin.sin_family = AF_INET;
  sin.sin_port = htons(cfg.port);

  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ <= 0) {
    throw std::runtime_error("socket() failed, error code: " +
                             std::to_string(errno));
  }

  if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&sin), sizeof(sin))) {
    std::string msg = (errno == 98)
                          ? ": Port in use or not released (can happen if "
                            "server process is just terminated"
                          : "";
    throw std::runtime_error(
        "bind() failed, error code: " + std::to_string(errno) + msg);
  }

  if (!SetNonblocking(listen_fd_)) {
    throw std::runtime_error("SetNonBlocking() failed, error code: " +
                             std::to_string(errno));
  }

  if (listen(listen_fd_, SOMAXCONN) == -1) {
    throw std::runtime_error("listen() failed, error code: " +
                             std::to_string(errno));
  }

  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    throw std::runtime_error("epoll_create1() failed, error code: " +
                             std::to_string(errno));
  }

  epoll_event e_event;
  e_event.events = EPOLLIN;
  e_event.data.fd = listen_fd_;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &e_event) == -1) {
    throw std::runtime_error("epoll_ctl() failed, error code: " +
                             std::to_string(errno));
  }
}

void Server::AddRoute(vector<string> allowed_methods, string pattern,
                      Handler handler) {
  Route route{allowed_methods, pattern, handler};
  routes_.emplace_back(route);
}

void Server::Listen() {
  size_t n = cfg_.num_threads;
  // size_t seq_num = 0;
  for (size_t i = 0; i < n; ++i) {
    auto ptr = std::make_shared<ServerThread>(listen_fd_, epoll_fd_, logger_,
                                              cfg_, routes_);
    server_threads_.emplace_back(ptr);
    threads_.emplace_back(std::thread(std::ref(*ptr)));
    ;
  }

  // Will run forever
  for (size_t i = 0; i < n; ++i) {
    threads_[i].join();
  }
}
