load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
    name = "refresh_compile_commands",
)

cc_library(
    name = "json_utils",
    srcs = ["json_utils.cc"],
    hdrs = ["json_utils.h"],
    deps = ["@com_google_protobuf//:protobuf"],
)

cc_library(
    name = "proto_io",
    srcs = ["proto_io.cc"],
    hdrs = ["proto_io.h"],
    deps = [
        ":json_utils",
        "@hyw_proto//proto/sim:map_cc_proto",
        "@hyw_proto//proto/sim:scenario_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "scenario_loader",
    srcs = ["scenario_loader.cc"],
    hdrs = ["scenario_loader.h"],
    deps = [
        ":proto_io",
        "@hyw_proto//proto/sim:map_cc_proto",
        "@hyw_proto//proto/sim:scenario_cc_proto",
    ],
)

cc_library(
    name = "geometry",
    srcs = ["geometry.cc"],
    hdrs = ["geometry.h"],
)

cc_library(
    name = "lane_graph",
    srcs = ["lane_graph.cc"],
    hdrs = ["lane_graph.h"],
    deps = [
        ":proto_io",
        "@hyw_proto//proto/sim:map_cc_proto",
        "@hyw_proto//proto/sim:runtime_cc_proto",
        "@hyw_proto//proto/sim:scenario_cc_proto",
    ],
)

cc_library(
    name = "map_query",
    srcs = ["map_query.cc"],
    hdrs = ["map_query.h"],
    deps = [
        ":lane_graph",
        "@hyw_proto//proto/sim:runtime_cc_proto",
    ],
)

cc_library(
    name = "trajectory_tracker",
    srcs = ["trajectory_tracker.cc"],
    hdrs = ["trajectory_tracker.h"],
    deps = [
        "@hyw_proto//proto/sim:runtime_cc_proto",
    ],
)

cc_library(
    name = "planner",
    hdrs = ["planner.h"],
    deps = [
        "@hyw_proto//proto/sim:runtime_cc_proto",
    ],
)

cc_library(
    name = "planner_process",
    srcs = ["planner_process.cc"],
    hdrs = ["planner_process.h"],
)

cc_library(
    name = "planner_client",
    srcs = ["planner_client.cc"],
    hdrs = ["planner_client.h"],
    deps = [
        ":planner",
        "@com_github_grpc_grpc//:grpc++",
        "@hyw_proto//proto/planner:planner_cc_grpc",
    ],
)

cc_library(
    name = "world",
    srcs = ["world.cc"],
    hdrs = ["world.h"],
    deps = [
        ":lane_graph",
        ":map_query",
        ":planner",
        ":trajectory_tracker",
        "@hyw_proto//proto/sim:runtime_cc_proto",
        "@hyw_proto//proto/sim:scenario_cc_proto",
    ],
)

cc_library(
    name = "grading_convert",
    srcs = ["grading_convert.cc"],
    hdrs = ["grading_convert.h"],
    deps = [
        "@hyw_proto//proto/sim:map_cc_proto",
        "@hyw_proto//proto/sim:runtime_cc_proto",
        "@hyw_proto//proto/grading:metric_input_cc_proto",
    ],
)

cc_library(
    name = "grading_bridge",
    srcs = ["grading_bridge.cc"],
    hdrs = ["grading_bridge.h"],
    deps = [
        ":grading_convert",
        "@hyw_proto//proto/sim:map_cc_proto",
        "@hyw_proto//proto/sim:runtime_cc_proto",
        "@com_google_protobuf//:protobuf",
        "@hyw_proto//proto/grading:metric_input_cc_proto",
        "@hyw_proto//proto/grading:sim_log_cc_proto",
    ],
)

cc_library(
    name = "sim_logger",
    srcs = ["sim_logger.cc"],
    hdrs = ["sim_logger.h"],
    deps = [
        "@hyw_proto//proto/sim:runtime_cc_proto",
        "@com_github_gabime_spdlog//:spdlog",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_binary(
    name = "sim_runner",
    srcs = ["main.cc"],
    deps = [
        ":grading_bridge",
        ":lane_graph",
        ":planner_client",
        ":planner_process",
        ":scenario_loader",
        ":sim_logger",
        ":world",
        "@hyw_proto//proto/sim:runtime_cc_proto",
    ],
)
