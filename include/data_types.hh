#pragma once

#include <vector>
#include <functional>

struct Header {
  std::string name;
  std::string value;
};


typedef std::vector<Header> Headers;


struct Request {
  std::string method;
  std::string uri;
  Headers headers;
  std::string content;  
  bool valid;
};


struct Response {
  int return_code;
  Headers headers;
  std::string content;
};


typedef std::function<void(const Request&, Response&)> Handler;


struct Route {
  std::vector<std::string> allowed_methods;
  std::string pattern;
  Handler handler;
};


typedef std::vector<Route> Routes;


