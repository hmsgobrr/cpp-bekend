#pragma once

#include <string>
#include <functional>

class HttpSocket {
public:
  int Listen(int port, std::function<std::string(std::string)> requestCallback);
private:
  int m_port = 8080;
  uint64_t m_socket = 0;
};
