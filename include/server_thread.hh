#pragma once

#define EPOLL_EVENT_ARRAY_SZ 64

#include <memory>
#include <mutex>
#include <unordered_map>

#include "connection.hh"
#include "data_types.hh"
#include "utils.hh"

class ServerThread {
 public:
  ServerThread(int listen_fd, int epoll_listen_fd,
               std::shared_ptr<Logger> logger, const ServerConfig& cfg,
               std::vector<Route> routes);

  void operator()();

  void AcceptConnection();

  void ProcessEvent(const epoll_event& ev);

  inline bool Timeout(Connection* conn, uint32_t time_now);

  void CloseConnection(int client_fd);

 private:
  const int listen_fd_;
  const int epoll_listen_fd_;
  std::shared_ptr<Logger> logger_;
  const ServerConfig& cfg_;
  Routes routes_;
  int epoll_fd_;
  epoll_event events_[EPOLL_EVENT_ARRAY_SZ];
  // map from client_fd to Connection obj
  std::unordered_map<int, Connection*> conn_map_;
  // shared buffer to receive data from client
  std::unique_ptr<char[]> buffer_;
};