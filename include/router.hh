#pragma once

typedef std::function<void(const Request&, Response&)> Handler;

struct Router {
  vector<string> allowed_methods;
  string uri;
  Handler handler;
};