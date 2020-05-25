#include <sstream>

#include "data_types.hh"
#include "defaults.hh"

namespace http_parser {

// find first occurence of c in buffer, search from (start + 1)
// return len if not found
size_t SeekToChar(size_t start, size_t len, const char* buffer, char c) {
  for (size_t i = start + 1; i < len; ++i) {
    if (buffer[i] == c) {return i;}
  }
  return len;
}


// TODO: write tests to make sure crash does not happen
Request ParseHttpRequest(const char* buffer, size_t len) {
  auto req = Request();
  req.valid = false;

  size_t start = 0;
  size_t end = SeekToChar(start, len, buffer, ' ');
  if (end >= len) return req;
  req.method = std::string(buffer + start, end - start);

  start = ++end;
  end = SeekToChar(start, len, buffer, ' ');
  if (end >= len) return req;
  req.uri = std::string(buffer + start, end - start);

  // Ignore http version
  start = SeekToChar(start, len, buffer, '\n') + 1;

  req.valid = true;
  while (true) {
    if (start < len && buffer[start] == '\r') {break;}
    if (start >= len) return req;
    
    Header header;

    end = SeekToChar(start, len, buffer, ' ');
    header.name = std::string(buffer + start, end - start - 1);

    start = ++end;
    if (start >= len) return req;
    end = SeekToChar(start, len, buffer, '\n');
    header.value = std::string(buffer + start, end - start - 1);

    req.headers.emplace_back(header);
    start = ++end;
  }
  start += 2;
  req.content = std::string(buffer + start, len - start);
  return req;
};


std::string ConstructHttpResponse(const Response& res) {
  std::stringstream ss;

  auto itr = defaults::http_code_to_reason.find(res.return_code);
  if (itr == defaults::http_code_to_reason.end()) {
    throw std::runtime_error("Invalid return code " + std::to_string(res.return_code));
  }
  ss << "HTTP/1.1 " << res.return_code << " " << itr->second << "\n";

  for (auto& header : res.headers) {
    ss << header.name << ": " << header.value << "\n";
  }

  ss << "\n" << res.content;

  return ss.str();
}

}



// GET /api/hi HTTP/1.1
// Host: localhost:1337
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.122 Safari/537.36
// Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
// Sec-Fetch-Site: none
// Sec-Fetch-Mode: navigate
// Sec-Fetch-User: ?1
// Sec-Fetch-Dest: document
// Accept-Encoding: gzip, deflate, br
// Accept-Language: vi,en-US;q=0.9,en;q=0.8


// HTTP/1.1 200 OK
// Content-Type: text/html; charset=iso-8859-1

// 1