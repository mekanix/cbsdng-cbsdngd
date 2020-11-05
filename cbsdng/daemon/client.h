#pragma once

#include "../message.h"


class Client
{
public:
  Client(const int &client);
  ~Client();

  const Message read();
  bool write(const uint32_t &size);
  bool write(const std::string &data);
  const int & raw() const;
  void cleanup();

protected:
  int fd;
};

Client &operator<<(Client &client, const Message &message);
Client &operator>>(Client &client, Message &message);
