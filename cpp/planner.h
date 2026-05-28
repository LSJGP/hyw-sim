#pragma once

#include <memory>
#include <string>

#include "proto/sim/runtime.pb.h"

namespace hyw_sim {

class Planner {
 public:
  virtual ~Planner() = default;
  virtual std::string Name() const = 0;
  virtual proto::PlannerTrajectory Plan(const proto::PlannerObservation& obs) const = 0;
};

}  // namespace hyw_sim
