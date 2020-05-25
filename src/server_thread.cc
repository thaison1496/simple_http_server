#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "server_thread.hh"

ServerThread::ServerThread(int listen_fd, int epoll_listen_fd,
                           std::shared_ptr<Logger> logger,
                           const ServerConfig& cfg, std::vector<Route> routes)
    : listen_fd_(listen_fd),
      epoll_listen_fd_(epoll_listen_fd),
      logger_(logger),
      cfg_(cfg),
      routes_(routes) {
  buffer_ = std::unique_ptr<char[]>(new char[cfg_.buffer_size]);
  epoll_fd_ = epoll_create1(0);
  if (epoll_fd_ == -1) {
    throw std::runtime_error("epoll_create1() failed, error code: " +
                             std::to_string(errno));
  }
}

void ServerThread::operator()() {
  while (true) {
    int num_events1 =
        epoll_wait(epoll_listen_fd_, events_, EPOLL_EVENT_ARRAY_SZ, 1000);
    for (int i = 0; i < num_events1; i++) {
      AcceptConnection();
    }

    int num_events2 =
        epoll_wait(epoll_fd_, events_, EPOLL_EVENT_ARRAY_SZ, 1000);
    if (num_events2 == -1) {
      continue;
    }

    // iterate signaled fds
    for (int i = 0; i < num_events2; i++) {
      logger_->Log("new event on fd: %d\n", events_[i].data.fd);
      ProcessEvent(events_[i]);
    }

    // Clear timed-out connections
    // Estimation, only call get time once
    uint32_t time_now = time(nullptr);
    for (auto itr = conn_map_.begin(); itr != conn_map_.end();) {
      auto& conn = itr->second;
      if (Timeout(conn, time_now)) {
        logger_->Log("server close fd: %d\n", conn->fd_);
        close(conn->fd_);
        delete conn;
        itr = conn_map_.erase(itr);
      } else {
        ++itr;
      }
    }
  }
}

void ServerThread::AcceptConnection() {
  sockaddr_in client_sin;
  socklen_t sin_size = sizeof(client_sin);

  int client_fd =
      accept(listen_fd_, reinterpret_cast<sockaddr*>(&client_sin), &sin_size);

  if (client_fd == -1) {
    logger_->Log("accept() failed, error code: %d\n", errno);
    return;
  }

  if (!SetNonblocking(client_fd)) {
    logger_->Log("failed to put fd into non-blocking mode, error code: %d\n",
                 errno);
    return;
  }

  epoll_event ev;
  // client events will be handled in edge-triggered mode
  ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;

  // initialize new connection
  auto conn =
      new Connection(client_fd, buffer_.get(), cfg_.buffer_size, routes_);
  ev.data.ptr = conn;
  conn_map_[client_fd] = conn;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) == 1) {
    logger_->Log("epoll_ctl() failed, error code: %d\n", errno);
    CloseConnection(client_fd);
    return;
  }

  logger_->Log("new client: %s:%d %d\n", inet_ntoa(client_sin.sin_addr),
               ntohs(client_sin.sin_port), client_fd);
}

void ServerThread::ProcessEvent(const epoll_event& ev) {
  auto conn = reinterpret_cast<Connection*>(ev.data.ptr);
  if (!conn->HandleEvent(ev)) {
    logger_->Log("client close fd: %d\n", conn->fd_);
    CloseConnection(conn->fd_);
  }
}

inline bool ServerThread::Timeout(Connection* conn, uint32_t time_now) {
  return conn->last_active_ + cfg_.timeout_secs < time_now;
}

void ServerThread::CloseConnection(int client_fd) {
  auto itr = conn_map_.find(client_fd);
  if (itr == conn_map_.end()) {
    return;
  }
  delete itr->second;
  conn_map_.erase(itr);
  close(client_fd);
}