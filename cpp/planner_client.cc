#include "cpp/planner_client.h"

#include <chrono>

#include "grpcpp/grpcpp.h"
#include "proto/planner/planner_service.grpc.pb.h"

namespace hyw_sim {
namespace {

std::shared_ptr<grpc::Channel> MakeChannel(const std::string& address) {
  return grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
}

void SetDeadline(grpc::ClientContext* ctx, int timeout_ms) {
  if (timeout_ms > 0) {
    ctx->set_deadline(std::chrono::system_clock::now() +
                      std::chrono::milliseconds(timeout_ms));
  }
}

}  // namespace

struct GrpcPlannerClient::Impl {
  std::shared_ptr<grpc::Channel> channel;
  std::unique_ptr<hyw_planner::proto::PlannerService::Stub> stub;
  std::string session_id;
  std::string name;
  int timeout_ms = 200;
};

GrpcPlannerClient::GrpcPlannerClient(std::string address, std::string session_id,
                                     std::string name, int timeout_ms)
    : impl_(std::make_unique<Impl>()) {
  impl_->channel = MakeChannel(address);
  impl_->stub = hyw_planner::proto::PlannerService::NewStub(impl_->channel);
  impl_->session_id = std::move(session_id);
  impl_->name = std::move(name);
  impl_->timeout_ms = timeout_ms;
}

GrpcPlannerClient::~GrpcPlannerClient() { CloseSession(); }

void GrpcPlannerClient::CloseSession() const {
  if (!impl_ || impl_->session_id.empty() || !impl_->stub) {
    return;
  }
  hyw_planner::proto::CloseSessionRequest req;
  req.set_session_id(impl_->session_id);
  hyw_planner::proto::CloseSessionResponse resp;
  grpc::ClientContext ctx;
  impl_->stub->CloseSession(&ctx, req, &resp);
  impl_->session_id.clear();
}

bool GrpcPlannerClient::HealthCheck(const std::string& address, std::string* error) {
  auto channel = MakeChannel(address);
  auto stub = hyw_planner::proto::PlannerService::NewStub(channel);
  hyw_planner::proto::HealthRequest req;
  hyw_planner::proto::HealthResponse resp;
  grpc::ClientContext ctx;
  SetDeadline(&ctx, 3000);
  const grpc::Status st = stub->Health(&ctx, req, &resp);
  if (!st.ok()) {
    if (error) {
      *error = "planner health check failed: " + st.error_message();
    }
    return false;
  }
  if (!resp.ok()) {
    if (error) {
      *error = "planner health check returned not ok";
    }
    return false;
  }
  return true;
}

std::unique_ptr<GrpcPlannerClient> GrpcPlannerClient::Connect(
    const std::string& address, const std::string& planner_name,
    const proto::PlannerInputs& inputs, int timeout_ms, std::string* error) {
  if (!HealthCheck(address, error)) {
    return nullptr;
  }

  auto channel = MakeChannel(address);
  auto stub = hyw_planner::proto::PlannerService::NewStub(channel);
  hyw_planner::proto::CreateSessionRequest req;
  req.set_planner_name(planner_name);
  *req.mutable_inputs() = inputs;
  hyw_planner::proto::CreateSessionResponse resp;
  grpc::ClientContext ctx;
  SetDeadline(&ctx, 10000);
  const grpc::Status st = stub->CreateSession(&ctx, req, &resp);
  if (!st.ok()) {
    if (error) {
      *error = "CreateSession failed: " + st.error_message();
    }
    return nullptr;
  }
  return std::unique_ptr<GrpcPlannerClient>(new GrpcPlannerClient(
      address, resp.session_id(), resp.planner_name(), timeout_ms));
}

std::string GrpcPlannerClient::Name() const { return impl_->name; }

proto::PlannerTrajectory GrpcPlannerClient::Plan(
    const proto::PlannerObservation& obs) const {
  hyw_planner::proto::PlanRequest req;
  req.set_session_id(impl_->session_id);
  *req.mutable_observation() = obs;
  hyw_planner::proto::PlanResponse resp;
  grpc::ClientContext ctx;
  SetDeadline(&ctx, impl_->timeout_ms);
  const grpc::Status st = impl_->stub->Plan(&ctx, req, &resp);
  if (!st.ok()) {
    throw std::runtime_error("Plan RPC failed: " + st.error_message());
  }
  return resp.trajectory();
}

}  // namespace hyw_sim
