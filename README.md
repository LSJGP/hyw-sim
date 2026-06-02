# sim — C++ 闭环仿真（Bazel）

当前主链路已迁移到 C++（`sim/cpp/sim_runner`）并使用 Bazel 构建：

1. 读取 Waymo 导出的 JSON 或 Proto 场景（`scenario_meta` / `dynamic_objects` / `lane_graph`）；
2. 在 C++ world loop 中通过 gRPC 调用独立 `planner_server`、车辆模型、OBB/SAT 碰撞与豁免分类；
3. 每帧将 `MetricFrameInput` JSON **入队**，由后台线程写入 `grading_main --stream` 的 stdin（仿真线程不等待 pipe / grading 速度；`Run` 结束后 `Finish` 会排空队列并关闭管道）。

场景与运行时数据由 `sim/proto/sim/*.proto` 定义；磁盘上默认 `--input-format=auto`：优先读取 `.pb`（否则回退 JSON）。其中 `lane_graph` 折线 `[[x,y,z],...]` 仍可走旧 JSON 路径。
4. 同时落盘 `sim_log.json`，可用于离线 batch 评分。

`run_sim.py` 仍保留为兼容入口，但内部会调用 C++ `sim_runner`。  
Python 仿真代码已独立到 `../hyw-workbench/pysim`。

## 目录

```
sim/
├── run_sim.py                      C++ 仿真兼容入口（转调 Bazel binary）
├── cpp/                            C++ 仿真实现
├── WORKSPACE                       Bazel workspace
└── .bazelrc                        Bazel build 配置
```

## 跑通四步

```bash
# 0. 已经有 scenarios/waymo_scenario_5/ 这个示例可以直接用；
#    要自己转新 scenario 见 ../hyw-workbench/tools/run_converter.sh

# 1. 编 hyw-planner 并启动 gRPC 服务（仿真前需常驻）
cd ../hyw-planner
bazel build //cpp:planner_server
bazel run //cpp:planner_server -- --port 50051 &
cd ../hyw-sim

# 2. 编一次 hyw-grading (只需第一次)
cd ../hyw-grading
PATH=$(echo "$PATH" | sed 's|/usr/lib/ccache:||g') CC=/usr/bin/gcc CXX=/usr/bin/g++ \
  bazel build //src/entry:grading_main \
  --action_env=PATH=/usr/local/bin:/usr/bin:/bin --action_env=CC=/usr/bin/gcc --action_env=CXX=/usr/bin/g++

# 3. 跑闭环仿真（sim_runner 经 gRPC 调 planner）
cd ../hyw-sim
python3 run_sim.py \
  --scenario-dir ../hyw-workbench/scenarios/waymo_scenario_5 \
  --output /tmp/sim_log.json \
  --grading-bin ../hyw-grading/bazel-bin/src/entry/grading_main \
  --grading-report /tmp/grading_report.json
  # 未指定 --metrics-config 时自动使用 ../hyw-grading/config/metrics_default.json
  # 默认 --cpp-mode online；其他: offline / both / off
  # 若使用 proto/streamload：可加 --input-format auto|proto|json 和 --scenario-load bulk|stream
```

## Proto/Streamload 快速验证

1. 生成 proto（可选 stream 分帧）：

```bash
# 在你的 TFRecord/数据转换环境中生成场景（示例）：
python3 ../hyw-workbench/tools/waymo_to_scenario.py \
  --tfrecord <path-to-tfrecord> \
  --out-dir ../hyw-workbench/scenarios/waymo_scenario_proto \
  --proto-only --split-dynamic-frames
```

2. 用 bulk 跑通（整包动态数据）：

```bash
python3 run_sim.py \
  --scenario-dir ../hyw-workbench/scenarios/waymo_scenario_proto \
  --output /tmp/sim_log.json \
  --input-format auto --scenario-load bulk
```

3. 用 stream 跑通（header+frames 按帧读盘）：

```bash
python3 run_sim.py \
  --scenario-dir ../hyw-workbench/scenarios/waymo_scenario_proto \
  --output /tmp/sim_log.json \
  --input-format auto --scenario-load stream
```

4. JSON 兼容：
如果你的场景仍只有 `scenario_meta.json/dynamic_objects.json/lane_graph.json`，保持默认 `--input-format auto` 继续可用。
