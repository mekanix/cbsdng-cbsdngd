#pragma once

#include <string>
#include <vector>


class Socket
{
public:
  Socket(const std::string &socketPath);
  ~Socket();

  void cleanup();
  int waitForClient();

protected:
  int fd;
  std::string socketPath;
};
