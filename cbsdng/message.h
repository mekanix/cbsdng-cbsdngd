#pragma once

#include <iostream>
#include <sstream>
#include <string>


enum Type
{
  NONE = 0x0,
  NOCOLOR = 0x1,
  JAIL = 0x2,
  BHYVE = 0x4,
  EXIT = 0x8,
};


class Message
{
public:
  Message()
    : _id{0}
    , _type{0}
    , _payload{""}
  {}
  Message(const int &id, const int &type, const std::string &payload)
    : _id{id}
    , _type{type}
    , _payload{payload}
  {}
  Message(const int &id, const int &type, const char &payload)
    : _id{id}
    , _type{type}
    , _payload{payload}
  {}

  friend std::ostream &operator<<(std::ostream &os, const Message &m)
  {
    os << m._id << ' ' << m._type << ' ' << m._payload;
    return os;
  }
  friend std::istream &operator>>(std::istream &is, Message &m)
  {
    is >> m._id;
    if (is.fail())
    {
      std::cerr << "Failed to load id\n";
      return is;
    }
    is >> m._type;
    if (is.fail())
    {
      std::cerr << "Failed to load type\n";
      return is;
    }
    char c;
    is.get(c);
    if (is.fail())
    {
      std::cerr << "Failed to load space after size\n";
      return is;
    }
    char buffer[1024];
    is.get(buffer, 1024, '\0');
    if (is.fail())
    {
      std::cerr << "Failed to load payload\n";
      return is;
    }
    m._payload += buffer;
    return is;
  }

  int id() const { return _id; };
  int type() const { return _type; };
  std::string payload() const { return _payload; };
  void id(const int &i) { _id = i; };
  void type(const int &t) { _type = t; };
  void payload(const std::string &p) { _payload = p; };
  void resize(const size_t &size) { _payload.resize(size); };

protected:
  int _id;
  int _type;
  std::string _payload;
};
