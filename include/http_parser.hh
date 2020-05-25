#pragma once

#include "data_types.hh"

namespace http_parser {

Request ParseHttpRequest(const char* buffer, size_t buffer_size);

std::string ConstructHttpResponse(const Response& res);

}