#pragma once

#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <queue>

#include "utils.hh"
#include "connection.hh"
#include "router.hh"
#include "server_thread.hh"

class Server {
public:
  Server(const ServerConfig& cfg);

  void Listen();

  void Route(
      std::vector<std::string> allowed_methods, 
      string uri,
      Handler handler);

private:
  void AcceptConnection();

  void CloseConnection(int client_fd);

  const ServerConfig& cfg_;
  int listen_fd_, epoll_fd_;
  std::shared_ptr<Logger> logger_;
  std::vector<Router> routes_;
  std::vector<std::shared_ptr<ServerThread> > server_threads_;
  std::vector<std::thread> threads_;
};
