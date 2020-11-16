#include <iostream>
#include <fstream>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/syslog_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "cbsdng/proxy/asyncworker.h"
#include "cbsdng/proxy/socket.h"


Socket *s;


void signalHandler(int sig)
{
  signal(sig, SIG_IGN);
  AsyncWorker::terminate();
  AsyncWorker::wait();
  s->cleanup();
  delete s;
  exit(0);
}


void ignoreSignal(int sig) { signal(sig, SIG_IGN); }

int main(int argc, char **argv)
{
  const auto optstr = "dp:s:";
  bool d = false;
  std::string socketPath = "/var/run/cbsdng/cbsdng.sock";
  std::string pidPath;
  int c;

  while ((c = getopt(argc, argv, optstr)) != -1) {
    switch (c) {
      case 'p':
        pidPath = optarg;
        break;
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

  s = new Socket(socketPath);

  signal(SIGINT, signalHandler);
  signal(SIGPIPE, ignoreSignal);

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

  if (pidPath.size() > 0)
  {
    auto mypid = getpid();
    std::ofstream pidfile;
    pidfile.open(pidPath);
    pidfile << getpid();
    pidfile.close();
  }
  // auto logger = spdlog::get("default");
  // logger->warn("Something wrong happened");

  while (1)
  {
    auto client = s->waitForClient();
    if (client != -1) { new AsyncWorker(client); }
    else { std::cerr << "Error accepting client!\n"; }
  }
  return 0;
}
