#pragma once

#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

#include "../message.h"
#include "client.h"
#include "socket.h"


class AsyncWorker
{
public:
  AsyncWorker(const int &client);
  ~AsyncWorker();

  static void removeFinished();
  static void terminate();
  static void wait();

  void execute(const Message &m);

protected:
  static bool quit;
  static std::mutex mutex;
  static std::condition_variable condition;
  static std::list<AsyncWorker *> finished;
  static bool checkFinished();

  Client client;
  std::thread t;

private:
  void _process();
};
