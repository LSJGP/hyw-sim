#pragma once

#include <memory>
#include <string>

namespace hyw_sim {

class PlannerProcess {
 public:
  PlannerProcess() = default;
  ~PlannerProcess();

  PlannerProcess(const PlannerProcess&) = delete;
  PlannerProcess& operator=(const PlannerProcess&) = delete;

  bool Start(const std::string& planner_bin, int port, std::string* error);
  std::string Address() const { return "127.0.0.1:" + std::to_string(port_); }

 private:
  int port_ = 50051;
  bool started_ = false;
};

}  // namespace hyw_sim
