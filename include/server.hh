#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "connection.hh"
#include "data_types.hh"
#include "server_thread.hh"
#include "utils.hh"

class Server {
 public:
  Server(const ServerConfig& cfg);

  void Listen();

  void AddRoute(std::vector<std::string> allowed_methods, string uri,
                Handler handler);

 private:
  void AcceptConnection();

  void CloseConnection(int client_fd);

  const ServerConfig& cfg_;
  int listen_fd_, epoll_fd_;
  std::shared_ptr<Logger> logger_;
  Routes routes_;
  std::vector<std::shared_ptr<ServerThread> > server_threads_;
  std::vector<std::thread> threads_;
};
