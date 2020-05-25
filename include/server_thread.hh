#pragma once

#define EPOLL_EVENT_ARRAY_SZ 1024

#include <memory>
#include <unordered_map>
#include <mutex> 

#include "utils.hh"
#include "connection.hh"

class ServerThread {
public:
  ServerThread(int listen_fd, int epoll_listen_fd, std::shared_ptr<Logger> logger, const ServerConfig& cfg);

  void operator() ();

  void AcceptConnection();

  void ProcessEvent(const epoll_event& ev);

  inline bool Timeout(Connection* conn, uint32_t time_now);

  void CloseConnection(int client_fd);

private:
  const int listen_fd_;
  const int epoll_listen_fd_;
  std::shared_ptr<Logger> logger_;
  const ServerConfig& cfg_;
  int epoll_fd_;
  epoll_event events_[EPOLL_EVENT_ARRAY_SZ];
  // map from client_fd to Connection obj
  std::unordered_map<int, Connection*> conn_map_;
  // shared buffer to receive data from client
  std::unique_ptr<char[]> buffer_;
  // protect conn_map_
  std::mutex mtx_;
};