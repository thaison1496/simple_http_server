#include "utils.hh"
#include "server.hh"

int main() {
  ServerConfig cfg;
  cfg.enable_log = false;
  cfg.timeout_secs_ = 1;
  Server svr(cfg);
  svr.Listen();
  return 0;
}