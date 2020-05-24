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
Date: Sun, 18 Oct 2012 10:36:20 GMT
Server: Apache/2.2.14 (Win32)
Content-Type: text/html; charset=iso-8859-1
Connection: Closed

<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">
<html>
<head>
   <title>400 Bad Request</title>
</head>
<body>
   <h1>Bad Request</h1>
   <p>Your browser sent a request that this server could not understand.</p>
   <p>The request line contained invalid characters following the protocol string.</p>
</body>
</html>)";
// std::cout << std::string(cstr) << std::endl;
    return std::string(cstr);
    // return std::string();
  }
};