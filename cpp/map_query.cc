#include "cpp/map_query.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace hyw_sim {
namespace {

constexpr double kDefaultHalfLaneWidth = 1.75;

double PointToSegmentDist(double px, double py, double x1, double y1, double x2,
                          double y2) {
  const double dx = x2 - x1;
  const double dy = y2 - y1;
  const double l2 = dx * dx + dy * dy;
  if (l2 < 1e-12) {
    return std::hypot(px - x1, py - y1);
  }
  const double t =
      std::max(0.0, std::min(1.0, ((px - x1) * dx + (py - y1) * dy) / l2));
  const double qx = x1 + t * dx;
  const double qy = y1 + t * dy;
  return std::hypot(px - qx, py - qy);
}

double MinPolylineDist(
    const google::protobuf::RepeatedPtrField<hyw_sim::proto::Vec3>& poly,
    double px, double py) {
  double best = std::numeric_limits<double>::infinity();
  if (poly.size() < 2) return best;
  for (int i = 0; i + 1 < poly.size(); ++i) {
    best = std::min(best, PointToSegmentDist(px, py, poly[i].x(), poly[i].y(),
                                             poly[i + 1].x(), poly[i + 1].y()));
  }
  return best;
}

bool ProjectOnClosestSegment(const hyw_sim::proto::Lane& lane, double px, double py,
                             double* lateral_offset, double* along_dist, int* seg_i,
                             double* foot_x, double* foot_y) {
  double best_d = std::numeric_limits<double>::infinity();
  bool found = false;
  for (int i = 0; i + 1 < lane.centerline_size(); ++i) {
    const double x1 = lane.centerline(i).x();
    const double y1 = lane.centerline(i).y();
    const double x2 = lane.centerline(i + 1).x();
    const double y2 = lane.centerline(i + 1).y();
    const double dx = x2 - x1;
    const double dy = y2 - y1;
    const double l2 = dx * dx + dy * dy;
    if (l2 < 1e-12) continue;
    const double t =
        std::max(0.0, std::min(1.0, ((px - x1) * dx + (py - y1) * dy) / l2));
    const double qx = x1 + t * dx;
    const double qy = y1 + t * dy;
    const double d = std::hypot(px - qx, py - qy);
    if (d < best_d) {
      best_d = d;
      const double seg_len = std::sqrt(l2);
      const double cross = dx * (py - y1) - dy * (px - x1);
      *lateral_offset = cross / seg_len;
      *along_dist = t * seg_len;
      if (seg_i != nullptr) {
        *seg_i = i;
      }
      if (foot_x != nullptr) {
        *foot_x = qx;
      }
      if (foot_y != nullptr) {
        *foot_y = qy;
      }
      found = true;
    }
  }
  return found;
}

const google::protobuf::RepeatedPtrField<hyw_sim::proto::Vec3>*
ResolveBoundaryPolyline(const LaneGraph& graph, const proto::Lane& lane, bool is_left,
                        int seg_i) {
  const auto& bounds = is_left ? lane.left_boundaries() : lane.right_boundaries();
  for (const auto& seg : bounds) {
    if (seg_i < seg.lane_start_index() || seg_i > seg.lane_end_index()) {
      continue;
    }
    const auto* poly = graph.FindFeaturePolyline(seg.boundary_feature_id());
    if (poly != nullptr && poly->size() >= 2) {
      return poly;
    }
  }
  return nullptr;
}

}  // namespace

proto::RoadContext BuildRoadContext(const LaneGraph& graph,
                                    const proto::VehicleState& ego) {
  proto::RoadContext out;
  const double x = ego.x();
  const double y = ego.y();

  const proto::Lane* lane =
      graph.ClosestLane(x, y, ego.heading(), true, M_PI / 2.0);
  double lateral = 0.0;
  if (lane != nullptr) {
    out.set_closest_lane_id(lane->id());
    double along = 0.0;
    int seg_i = -1;
    double foot_x = 0.0;
    double foot_y = 0.0;
    if (ProjectOnClosestSegment(*lane, x, y, &lateral, &along, &seg_i, &foot_x,
                                  &foot_y)) {
      out.set_lateral_offset_m(lateral);

      const auto* left_poly =
          ResolveBoundaryPolyline(graph, *lane, true, seg_i);
      const auto* right_poly =
          ResolveBoundaryPolyline(graph, *lane, false, seg_i);

      double w_left = kDefaultHalfLaneWidth;
      double w_right = kDefaultHalfLaneWidth;
      if (left_poly != nullptr) {
        const double d = MinPolylineDist(*left_poly, foot_x, foot_y);
        if (std::isfinite(d)) {
          w_left = d;
        }
      }
      if (right_poly != nullptr) {
        const double d = MinPolylineDist(*right_poly, foot_x, foot_y);
        if (std::isfinite(d)) {
          w_right = d;
        }
      }

      out.set_dist_to_left_boundary_m(w_left - lateral);
      out.set_dist_to_right_boundary_m(w_right + lateral);
    }
  }

  double min_edge = std::numeric_limits<double>::infinity();
  for (const auto& edge : graph.map().road_edges()) {
    min_edge = std::min(min_edge, MinPolylineDist(edge.polyline(), x, y));
  }
  for (const auto& line : graph.map().road_lines()) {
    min_edge = std::min(min_edge, MinPolylineDist(line.polyline(), x, y));
  }
  if (std::isfinite(min_edge)) {
    out.set_dist_to_road_edge_m(min_edge);
  }

  return out;
}

}  // namespace hyw_sim
