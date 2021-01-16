#include <iostream>
#include <string>
#include <vector>
#include <signal.h>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/event.h>
#include <libutil.h>
#include <termios.h>
#include <spdlog/spdlog.h>

#include "cbsdng/proxy/asyncworker.h"


#define READ_END 0
#define WRITE_END 1


bool AsyncWorker::quit = false;
std::mutex AsyncWorker::mutex;
std::condition_variable AsyncWorker::condition;
std::list<AsyncWorker *> AsyncWorker::finished;
static auto finishedThread = std::thread(&AsyncWorker::removeFinished);
static bool parent = true;


AsyncWorker::AsyncWorker(const int &cl)
  : client{cl}
{
  t = std::thread(&AsyncWorker::_process, this);
}


void AsyncWorker::execute(const Message &m)
{
  auto logger = spdlog::get("default");
  int noc = m.type() & Type::NOCOLOR;
  if (noc == 1)
  {
    std::stringstream nocolor;
    nocolor << noc;
    const auto data = nocolor.str();
    if (data.size() > 0) { setenv("NOCOLOR", data.data(), 1); }
  }
  int bhyve = m.type() & Type::BHYVE;
  if (bhyve > 0) { prefix = 'b'; }

  pid = forkpty(&child, nullptr, nullptr, nullptr);
  if (pid < 0) // forking or pty failed
  {
    logger->error("Failed to fork()");
    return;
  }
  else if (pid == 0) { executeCBSD(m); }
  else { executeProxy(m.id()); }
}


void AsyncWorker::executeCBSD(const Message &m)
{
  parent = false;
  std::vector<std::string> args;
  std::string command = prefix + m.payload();
  std::string raw_command = "cbsd " + command;
  char *token = strtok((char *)raw_command.data(), " ");
  args.push_back(token);
  while ((token = strtok(nullptr, " ")) != nullptr)
  {
    args.push_back(token);
  }
  char *raw_args[args.size()];
  for (int i = 0; i < args.size(); ++i)
  {
    raw_args[i] = (char *)args[i].data();
  }
  execvp(raw_args[0], raw_args);
  terminate();
  wait();
  return;
}


void AsyncWorker::executeProxy(const int &id)
{
  auto logger = spdlog::get("default");
  struct kevent events[2];
  struct kevent tevent;
  int kq = kqueue();
  if (kq == -1) { std::cerr << "kqueue: " << strerror(errno) << '\n'; }
  EV_SET(events, child, EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_READ, 0, nullptr);
  EV_SET(events + 1, client.raw(), EVFILT_READ, EV_ADD | EV_CLEAR, NOTE_READ, 0, nullptr);
  int rc = kevent(kq, events, 2, nullptr, 0, nullptr);
  if (rc == -1)
  {
    std::cerr << "kevent register: " << strerror(errno) << '\n';
  }
  while (true)
  {
    rc = kevent(kq, nullptr, 0, &tevent, 1, nullptr);
    if (rc == -1 || tevent.data == 0) { break; }
    if (tevent.ident == child)
    {
      char buffer[tevent.data + 1];
      rc = read(tevent.ident, buffer, tevent.data);
      if (rc <= 0) { continue; }
      buffer[rc] = '\0';
      Message output(id, 0, buffer);
      client << output;
    }
    else
    {
      Message input;
      client >> input;
      const auto &payload = input.payload();
      write(child, payload.data(), payload.size());
    }
  }
  int st;
  waitpid(pid, &st, 0);
  std::stringstream s;
  s << st;
  Message output(id, Type::EXIT, s.str());
  client << output;
}


void AsyncWorker::_process()
{
  Message m;
  client >> m;
  execute(m);
  if (parent)
  {
    std::unique_lock<std::mutex> lock(mutex);
    finished.push_back(this);
    lock.unlock();
    condition.notify_one();
  }
}


void AsyncWorker::removeFinished()
{
  while (true)
  {
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, &AsyncWorker::checkFinished);
    if (!parent) { return; }
    auto worker = finished.front();
    finished.pop_front();
    if (worker != nullptr) { delete worker; }
    if (quit && finished.empty()) { return; }
  }
}


void AsyncWorker::terminate()
{
  quit = true;
  std::unique_lock<std::mutex> lock(mutex);
  finished.push_back(nullptr);
  lock.unlock();
  condition.notify_all();
}


bool AsyncWorker::checkFinished() { return !finished.empty(); }
void AsyncWorker::wait() { finishedThread.join(); }
AsyncWorker::~AsyncWorker() { t.join(); }
