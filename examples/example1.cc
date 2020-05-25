#include <sstream>

#include "server.hh"

int main() {
  ServerConfig cfg;
  // cfg.enable_log = true;
  // cfg.timeout_secs = 1;
  // cfg.num_threads = 1;
  // cfg.logger = "log.txt"; // log make server much slower

  cfg.enable_log = false;
  cfg.timeout_secs = 1;
  cfg.num_threads = 20;

  Server svr(cfg);

  svr.AddRoute({"GET"}, "/hello", [](const Request& req, Response& res) {
    res.content = "<h1>Hello<h1>";
  });

  svr.AddRoute({"GET", "POST"}, "/echo", [](const Request& req, Response& res) {
    std::stringstream ss;
    ss << "<pre>Echo headers: \n";
    for (auto& header : req.headers) {
      ss << header.name << ": " << header.value << "\n";
    }
    ss << "\nEcho content: \n</pre>" << req.content;
    res.content = ss.str();
  });

  svr.Listen();
  return 0;
}