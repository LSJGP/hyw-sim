#include "cpp/planner_process.h"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>

namespace hyw_sim {
namespace {

pid_t g_planner_child = -1;

void KillChildIfRunning() {
  if (g_planner_child > 0) {
    kill(g_planner_child, SIGTERM);
    waitpid(g_planner_child, nullptr, 0);
    g_planner_child = -1;
  }
}

}  // namespace

PlannerProcess::~PlannerProcess() {
  KillChildIfRunning();
  started_ = false;
}

bool PlannerProcess::Start(const std::string& planner_bin, int port, std::string* error) {
  if (started_) {
    return true;
  }
  port_ = port;
  const pid_t pid = fork();
  if (pid < 0) {
    if (error) {
      *error = "fork failed for planner_server";
    }
    return false;
  }
  if (pid == 0) {
    std::ostringstream port_arg;
    port_arg << port_;
    execl(planner_bin.c_str(), planner_bin.c_str(), "--port", port_arg.str().c_str(),
          "--address", "127.0.0.1", static_cast<char*>(nullptr));
    std::cerr << "[sim_cpp] failed to exec planner_server: " << planner_bin << "\n";
    _exit(127);
  }
  g_planner_child = pid;
  started_ = true;
  return true;
}

}  // namespace hyw_sim
