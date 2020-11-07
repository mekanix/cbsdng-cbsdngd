#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "cbsdng/daemon/asyncworker.h"
#include "cbsdng/daemon/socket.h"


Socket *s;


void signalHandler(int sig)
{
  signal(sig, SIG_IGN);
  AsyncWorker::terminate();
  AsyncWorker::wait();
  s->cleanup();
  exit(0);
}


void ignoreSignal(int sig)
{
  signal(sig, SIG_IGN);
}

int main(int argc, char **argv)
{
  const auto optstr = "ds:";
  bool d = false;
  std::string socketPath = "/var/run/cbsdng/cbsdng.sock";
  int c;

  signal(SIGINT, signalHandler);
  signal(SIGPIPE, ignoreSignal);

  while ((c = getopt(argc, argv, optstr)) != -1) {
    switch (c) {
      case 's':
        socketPath = optarg;
        break;
      case 'd':
        d = true;
        break;
      default:
        return 1;
    }
  }

  if (d)
  {
    daemon(0, 0);
    spdlog::syslog_logger_mt("default");
  }
  else
  {
    spdlog::stdout_color_mt("default");
  }
  s = new Socket(socketPath);
  // auto logger = spdlog::get("default");
  // logger->warn("Something wrong happened");

  while (1)
  {
    auto client = s->waitForClient();
    if (client != -1)
    {
      new AsyncWorker(client);
    }
    else
    {
      std::cerr << "Error accepting client!\n";
    }
  }
  delete s;
  return 0;
}
