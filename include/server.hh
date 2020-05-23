#pragma once

#include <unistd.h>
#include <vector>
#include <unordered_map>

#include "connection.hh"
#include "utils.hh"

class Server {
public:
  Server(const ServerConfig& cfg);

  void Listen();

  void Route(
      std::vector<std::string> allowed_methods, 
      string uri,
      Handler handler);

  ~Server();

private:
  void AcceptConnection();

  void ProcessEvent(const epoll_event& ev);

  void CloseConnection(int client_fd);

  inline bool Timeout(Connection* conn, uint32_t time_now);

  template <typename... Args>
  void WriteLog(Args&&... args) {
    if (cfg_.enable_log) {
      printf(std::forward<Args>(args)...);
    }
  }

  const ServerConfig& cfg_;
  int listen_fd_, epoll_fd_;
  epoll_event* events_;
  // Map from client_fd to Connection obj
  std::unordered_map<int, Connection*> conn_map_;
  std::unique_ptr<Logger> logger_;
  // Shared read buffer, need 1 buffer for each thread
  char* buffer_;
  std::vector<Router> routes_;
};