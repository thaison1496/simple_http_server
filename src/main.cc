#include "utils.hh"
#include "server.hh"

int main() {
  ServerConfig cfg;
  cfg.enable_log = true;
  cfg.timeout_secs = 1;
  cfg.num_threads = 5;
  cfg.logger = "log.txt";
  Server svr(cfg);
  svr.Listen();
  return 0;
}