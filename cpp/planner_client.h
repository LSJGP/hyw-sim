#pragma once

#include <memory>
#include <string>

#include "cpp/planner.h"

namespace hyw_sim {

class GrpcPlannerClient : public Planner {
 public:
  ~GrpcPlannerClient() override;

  static std::unique_ptr<GrpcPlannerClient> Connect(
      const std::string& address, const std::string& planner_name,
      const proto::PlannerInputs& inputs, int timeout_ms, std::string* error);

  static bool HealthCheck(const std::string& address, std::string* error);

  std::string Name() const override;
  proto::PlannerTrajectory Plan(const proto::PlannerObservation& obs) const override;

 private:
  GrpcPlannerClient(std::string address, std::string session_id, std::string name,
                    int timeout_ms);

  void CloseSession() const;

  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace hyw_sim
