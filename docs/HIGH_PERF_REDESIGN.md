# 高性能重构设计（破坏性 API，性能优先）

本文档记录了 ArkZero 渲染器在“不考虑向后兼容、以性能为第一优先级”情况下的重构方案。
目的：最大化吞吐与最小化提交延迟，适用于对延迟极其敏感的实时场景（视频/AR/游戏等）。

---

## 1 概要
- 将热路径中的 Promise/await/NAPI 开销最小化或移除，改为 fire-and-forget 的提交语义。
- 引入显式的 buffer 所有权转移（transferBuffer/transferOwnership），使 native 直接接管内存并在完成后回收。
- 在 native 层实现专用渲染线程、ring descriptor、GPU fence/timeline 和多帧 in-flight 管线。
- 提供轻量事件（frameToken, onFrameComplete）用于统计与回收，而不是阻塞提交。

## 2 破坏性 API（示例）
- 构造器：
  ```ts
  new ArkZeroRenderer({ width, height, format, backend:'vulkan'|'gles', maxInflight: number })
  ```
- 所有权转移（零拷贝）：
  ```ts
  const bufHandle = renderer.transferBuffer(myArrayBuffer);
  // 转移后 JS 侧不应再访问 myArrayBuffer
  ```
- 提交帧（立即返回 token）：
  ```ts
  const token = renderer.submitFrame({ bufferHandle: bufHandle, width, height, timestamp });
  ```
- 轻量回调与等待：
  ```ts
  renderer.onFrameComplete((token, status)=>{ /* 回收或统计 */ });
  await renderer.fence(token, timeoutMs); // 可选阻塞等待
  await renderer.shutdown(timeoutMs); // 优雅关闭
  ```

## 3 JS 层实现要点
- 移除每帧返回 Promise 的热路径，`submitFrame` 直接返回 `frameToken`，不做阻塞。
- `transferBuffer` 负责把 ArrayBuffer 的指针交给 native；JS 失去对该内存的直接访问权以避免竞争。
- 提供可配置的流控策略：`DropOnOverflow`（丢帧）或 `BackPressure`（阻塞/返回错误），默认面向低延迟场景使用 `DropOnOverflow`。

## 4 Native 层实现要点
- 启动专用渲染线程（Render Worker），维护 ring descriptor queue（lock-free 或小锁实现）。
- 使用 `maxInflight` 个 staging texture/buffer 做环形复用，提交时分配 index 并设置 fence。
- 使用 GL sync 或 Vulkan timeline/fence 在 GPU 完成时回收对应 buffer，并批量回调 JS 层（减少 NAPI 次数）。
- 支持 batch submit：按阈值把若干 descriptor 合并成一次 driver 提交以减少开销。

## 5 内存与回收策略
- 预分配 buffer pool，尽量避免在运行时分配/释放。
- 显式所有权转移后，native 负责回收并通过回调/句柄重用信号通知 JS（或让 JS 从 pool 中获取新 buffer）。

## 6 同步与流控
- 默认非阻塞（fire-and-forget）；当 queue 满时根据策略丢帧或返回错误。
- 提供可选 fence 等待接口供需要严格帧交付保证的上层使用。

## 7 性能期望
- 提交路径延迟下降到微秒级（受限于 NAPI 最小序列化成本）；吞吐取决于 GPU 上传带宽与 `maxInflight`。
- 目标场景：低延迟实时渲染（目标 end-to-end <10ms，当硬件与驱动支持时）。

## 8 风险与权衡
- 破坏 JS 端的内存安全语义：错误使用（在转移后访问 buffer）会导致原生崩溃。
- 增加 native 复杂度：lock-free、fence 管理、批回调等都需要精心实现和大量回归测试。
- 需针对 Vulkan 与 GLES 做不同实现与调优。

## 9 测试矩阵（建议）
- 功能测试：submit/回收/回调/错误路径。
- 压力测试：持续高频提交，测最大 sustainable FPS、丢帧率、内存使用。
- 极端场景：频繁 shutdown、resize、同时来自多实例的提交。

## 10 迁移建议
- 逐步推出：先在 native 层做 PoC（内部开关），提供高级 API 作为实验特性；待稳定后再文档化并替换旧 API。

---

文件作者：自动生成（由 Copilot 协助）
日期：2026-05-26
