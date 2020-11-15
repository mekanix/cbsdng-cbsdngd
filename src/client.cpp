#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "cbsdng/proxy/client.h"


Client::Client(const int &cl) : fd{cl}
{}


const Message Client::read()
{
  Message m;
  uint32_t data;
  int rc = ::read(fd, &data, sizeof(data));
  if (rc <= 0)
  {
    m.type(-1);
    m.payload(strerror(errno));
    return m;
  }
  m.id(data);

  rc = ::read(fd, &data, sizeof(data));
  if (rc <= 0)
  {
    m.type(-1);
    m.payload(strerror(errno));
    return m;
  }
  m.type(data);

  uint32_t size;
  rc = ::read(fd, &size, sizeof(size));
  if (rc <= 0)
  {
    m.type(-1);
    m.payload(strerror(errno));
    return m;
  }
  std::string payload;
  payload.resize(size+1);
  rc = ::read(fd, (char *)payload.data(), size);
  if (rc <= 0)
  {
    m.type(-1);
    m.payload(strerror(errno));
    return m;
  }
  m.payload(payload);
  return m;
}


bool Client::write(const uint32_t &data)
{
  int rc = ::write(fd, &data, sizeof(data));
  if (rc <= 0) { return false; }
  return true;
}


bool Client::write(const std::string &data)
{
  int rc = ::write(fd, data.data(), data.size());
  if (rc <= 0) { return false; }
  return true;
}


void Client::cleanup()
{
  if (fd != -1)
  {
    close(fd);
    fd = -1;
  }
}


const int & Client::raw() const { return fd; }
Client::~Client() { cleanup(); }


Client &operator<<(Client &client, const Message &message)
{
  client.write(message.id());
  client.write(message.type());
  const auto &payload = message.payload();
  client.write(payload.size());
  client.write(payload);
  return client;
}


Client &operator>>(Client &client, Message &message)
{
  message = client.read();
  return client;
}
