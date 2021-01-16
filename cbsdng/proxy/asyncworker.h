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
  void executeCBSD(const Message &m);
  void executeProxy(const int &id);

protected:
  static bool quit;
  static std::mutex mutex;
  static std::condition_variable condition;
  static std::list<AsyncWorker *> finished;
  static bool checkFinished();

  Client client;
  std::thread t;
  char prefix = 'j';
  int child;
  int pid;

private:
  void _process();
};
