#pragma once

#include <cstdint>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "google/protobuf/repeated_ptr_field.h"
#include "proto/grading/metric_input.pb.h"
#include "proto/sim/map.pb.h"
#include "proto/sim/runtime.pb.h"
#include "proto/sim/scenario.pb.h"

namespace hyw_sim {

class LaneGraph {
 public:
  static bool LoadFromFile(const std::string& path, LaneGraph* out, std::string* error);

  explicit LaneGraph(proto::StaticMap map);

  const proto::Lane* FindLane(int64_t id) const;

  const proto::Lane* ClosestLane(double x, double y, double heading,
                                 bool has_heading = true,
                                 double max_heading_diff = 1.5707963268) const;

  std::vector<int64_t> ShortestPath(int64_t start_id, int64_t goal_id) const;

  std::vector<std::tuple<double, double, double>> RouteCenterline(
      const std::vector<int64_t>& lane_ids) const;

  double SpeedLimitMps(const std::vector<int64_t>& lane_ids,
                       double default_kmh = 50.0) const;

  size_t LaneCount() const { return static_cast<size_t>(map_.lanes_size()); }

  const proto::StaticMap& map() const { return map_; }

  // Returns polyline for a map feature id (road_line or road_edge), or nullptr.
  const google::protobuf::RepeatedPtrField<proto::Vec3>* FindFeaturePolyline(
      int64_t feature_id) const;

 private:
  void BuildFeaturePolylineIndex();

  proto::StaticMap map_;
  std::unordered_map<int64_t, const google::protobuf::RepeatedPtrField<proto::Vec3>*>
      feature_polylines_;
};

bool BuildMapReference(const proto::ScenarioMeta& meta, const LaneGraph& graph,
                       double reference_step, proto::MapRouteResult* out,
                       std::string* error);

std::vector<proto::ReferencePoint> BuildSdcReference(
    const proto::DynamicObjects& dynamic);

std::vector<proto::ReferencePoint> BuildSdcReferenceFromTrack(
    const proto::Track& sdc_track);

bool ExtractSdcRouteEndpoints(
    const proto::Track& sdc_track,
    grading_mini::proto::SdcRouteContext* out);

}  // namespace hyw_sim
