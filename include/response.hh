#pragma once

#include "utils.hh"
#include "header.hh"
#include <iostream>

struct Response {
  int return_code;
  Headers headers;
  std::string content;

  std::string ConstructResponse() {
    const char* cstr = R"(HTTP/1.1 200 OK
Content-Type: text/html; charset=iso-8859-1

1)";
// std::cout << std::string(cstr) << std::endl;
    return std::string(cstr);
    // return std::string();
  }
};