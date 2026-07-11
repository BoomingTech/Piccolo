# Piccolo 引擎学习路线图（全模块 · 渲染重点）

> 目标：从「只会写业务逻辑」进阶到「理解游戏引擎底层」。
> 路线：**先整体过一遍引擎全貌 → 分模块逐个深入 → 边读边改，改动推送到 GitHub**。
> 当前进度：已克隆并成功编译（Release 可用，亦可编 Debug），`bin/PiccoloEditor.exe` 可运行。

---

## 0. 学习方法与 Git 工作流

### 学习方法（每个模块固定四步循环）
1. **读源码**：带着问题读，先抓「这个模块管什么、输入是什么、输出去哪」。
2. **理数据流**：画出「数据从哪来、到哪去」。例：物理 = `rigidbody` 组件 → `physics_scene` → Jolt → 写回 `transform`。
3. **改一处**：哪怕只改一个参数 / 加一行日志，立刻编译看效果。
4. **提交**：每完成一个小改动就提交到对应分支并推 GitHub。

### Git 分支策略
- `main`：保持与上游一致，**不直接往 main 提交学习改动**。
- 每个模块一个分支：`learn/overview`、`learn/render`、`learn/physics`、`learn/animation` …
- 提交信息用中文，格式：`[模块] 做了什么`，如 `[physics] 加碰撞事件日志`。
- 改动先在分支积累，确认后再考虑合回 main（或不合，仅作学习记录）。

---

## 1. 阶段一：整体概览（先过一遍全引擎）

目标：钻进任何模块前，先建立「引擎长什么样、模块怎么连、一帧怎么跑完」的整体地图。

### 1.1 引擎架构鸟瞰
Piccolo 是**组件-系统（ECS 风格）**的延迟渲染引擎。核心分层：

- **core（地基）**：`math`（向量/矩阵/四元数）、`color`、`log`、`meta`（反射 + 序列化 + json）
- **function（功能）**：`framework`（ECS 核心）、`render`、`physics`、`animation`、`character`、`controller`、`input`、`particle`、`ui`、`global`
- **resource（资源）**：`asset_manager`（加载/缓存）、`config_manager`、`res_type`（资源类型）
- **platform**：`file_service`、`path`
- **editor**：`PiccoloEditor`（基于 ImGui，靠反射系统自动生成属性面板）
- **3rdparty**：VulkanSDK、JoltPhysics（物理）、glfw、imgui、lua、sol2、spdlog、stb、vulkanmemoryallocator

### 1.2 三个核心机制（必须理解）
1. **ECS 模式**：`Object` = 一组 `Component` 的容器；每种 Component 有对应的 `System`（`render_system`、`physics_manager`、`animation_system`…）；`world_manager` 每帧按序 tick 所有 system。
   - 文件：`function/framework/object/object.h`、`component/component.h`、`world/world_manager.cpp`
2. **Tick 主循环**：`engine.cpp` 的 `tick()` → 更新逻辑 → 各 system 更新 → 渲染提交。
   - 文件：`runtime/engine.cpp`
3. **反射系统（meta）**：Piccolo 的「灵魂」。通过 `core/meta/reflection` 在运行时知道类的字段，从而自动生成编辑器属性面板、做序列化、绑定 Lua。
   - 文件：`core/meta/reflection/reflection.h`、`reflection_register.h`、`meta_example.h`

### 1.3 一帧的生命周期
```
engine.tick()
  → input_system 收集输入
  → world_manager 遍历所有 object
      → 各 component 的 system 更新（physics 积分、animation 采样、controller 移动…）
  → render_system 收集可见物体
      → render_pipeline 跑延迟渲染管线 → 输出到屏幕
  → ui/editor 画 ImGui 面板
```

### 1.4 本阶段动手
- [ ] 读 `engine.cpp`、`world_manager.cpp`、`global_context.cpp`、`reflection.h`，各写 2 行注释说明它做什么。
- [ ] 在主循环加帧耗时日志（逻辑耗时 vs 渲染耗时），跑起来看数字。
- [ ] 画一张自己的引擎架构图（可用 level/object 当例子）。

---

## 2. 阶段二：分模块逐个深入

> 难度：★ 入门 / ★★ 中等 / ★★★ 进阶。渲染部分最细（你的重点），物理部分配了「它到底怎么算」的讲解（你的盲区）。

### 2.1 引擎地基 core（★）
- 关键文件：`core/math/*`、`core/meta/reflection/*`、`core/meta/serializer/*`、`core/meta/json.h`、`core/log/*`
- 核心概念：数学库（向量/矩阵/四元数/变换）、反射注册宏、序列化（对象 ↔ JSON）、日志。
- 动手：用反射系统给一个已有类加一个新字段，看它是否自动出现在编辑器面板 + 序列化里。

### 2.2 框架与 ECS（★）
- 关键文件：`framework/object/*`、`framework/component/*`（transform / mesh / camera / animation / lua / motor / particle / rigidbody）、`framework/level/*`、`framework/world/*`、`global/global_context.*`
- 核心概念：Object 如何持有 Component；System 如何遍历 Component；Level 是场景容器；World 驱动一切；GlobalContext 是模块间拿彼此入口的全局表。
- 动手：写一个新 Component（如 `health_component`），并让它被某个 system 每帧打印一次 —— 验证你理解了 ECS 套路。

### 2.3 渲染 render（★★★ 重点）
分 5 个子阶段：
- **RHI 抽象层**：`function/render/interface/rhi.h` + `rhi_struct.h`（图形接口）+ `interface/vulkan/*`（Vulkan 实现，含 VMA 显存分配）。
- **渲染管线**：`render_system.cpp` → `render_pipeline*.{h,cpp}` → `render_pass*.{h,cpp}`（基类）。
- **Pass 与 Shader**：`passes/*` 对照 `engine/shader/*` 的 GLSL（`main_camera_pass` 写 G-buffer；`directional_light_pass` / `point_light_pass` 光照；`tone_mapping_pass` / `color_grading_pass` / `fxaa_pass` 后处理；`combine_ui_pass` 叠 UI）。
- **场景与资源**：`render_scene`、`render_camera`、`render_entity`、`render_mesh`、`window_system`（GLFW + Vulkan Surface）。
- **动手（按收益排序）**：① 加渲染统计 HUD（draw call / 显存）；② 加开关跳过某后处理 pass；③ 改一个 shader 看画面变化；④ 视锥剔除减少 draw call。

### 2.4 物理 physics（★★ 你的盲区，重点讲）
Piccolo 用 **JoltPhysics**（开源刚体物理引擎，与 PhysX / Havok 同类）。

**它到底怎么算（一帧里物理做的事）：**
1. **数据接入**：`rigidbody_component` 定义刚体的形状（盒 / 球 / 胶囊）、质量、是否静态；`physics_scene` 据此在 Jolt 里创建一个 `Body`。
2. **积分 Integration**：根据受力 / 扭矩，用时间步更新每个刚体的速度、角速度、位置、朝向（常用半隐式欧拉 + 子步 substep 提高稳定性）。
3. **宽相 Broadphase**：用加速结构（BVH / Sweep & Prune）快速筛出「可能碰撞」的刚体对，避免 O(n²) 两两检测。
4. **窄相 Narrowphase**：对候选对做精确碰撞检测，算出接触点（contact point）、法线、穿透深度。
5. **约束求解 Solver**：迭代求解接触约束和关节约束，把物体「推开」到不穿透、并按物理规律反弹 / 滑动。这是物理稳定与否的关键。
6. **写回**：把算完的 transform 写回 `transform_component`，渲染和游戏逻辑就能读到新位置。

- 关键文件：`function/physics/physics_manager.{h,cpp}`、`physics_scene.{h,cpp}`、`physics_config.h`、`jolt/utils.{h,cpp}`、`framework/component/rigidbody/*`、`engine/3rdparty/JoltPhysics/`
- 核心概念：RigidBody、CollisionShape、Broadphase、Narrowphase、Contact、Constraint、Solver、Substep、Static vs Dynamic。
- 动手：① 在 `physics_scene` 加碰撞事件回调（A 碰到 B 打日志）；② 用 `debugdraw` 把碰撞体线框画出来，直观看物理形状；③ 调 `physics_config` 的 substep / 迭代次数，观察稳定性变化。

### 2.5 动画 animation（★★）
- 关键文件：`function/animation/animation_system.*`、`skeleton.*`、`node.*`、`animation_loader.*`、`utilities.*`、`component/animation/*`
- 核心概念：骨骼（Skeleton）、骨骼节点（Node）、动画采样、骨骼变换层级、动画混合（后续可深入）。
- 动手：加载一个已有动画，在编辑器里调播放速度 / 循环，观察 skeleton 节点变换如何驱动 mesh。

### 2.6 角色与控制器 character / controller（★★）
- 关键文件：`function/character/character.*`、`function/controller/character_controller.*`、`component/rigidbody/*`、`component/motor/*`
- 核心概念：角色 = 动画 + 控制器 + 刚体 的组合；角色控制器处理移动 / 跳跃 / 碰撞响应（常基于胶囊体 + 射线 / 扫掠）。
- 动手：调 `character_controller` 的移动速度 / 跳跃力，或给角色加一个简单的状态（走 / 跑）。

### 2.7 输入 input（★）
- 关键文件：`function/input/input_system.{h,cpp}`
- 核心概念：输入设备抽象、按键映射、帧内输入快照。
- 动手：加一个自定义按键，绑定到一个简单的编辑器动作。

### 2.8 粒子 particle（★★）
- 关键文件：`function/particle/particle_manager.*`、`particle_desc.*`、`particle_common.h`、`emitter_id_allocator.*`、`component/particle/*`、`render/passes/particle_pass.*`
- 核心概念：发射器（Emitter）、粒子描述、CPU / GPU 粒子更新、渲染如何画粒子。
- 动手：改粒子参数（数量 / 生命周期 / 速度），看画面变化。

### 2.9 UI 与编辑器 ui / editor（★★）
- 关键文件：`function/ui/window_ui.*`、`source/editor/*`（ImGui）、`core/meta`（属性面板靠反射）
- 核心概念：ImGui 即时模式 UI、编辑器如何靠反射自动生成组件属性面板、关卡保存 / 加载。
- 动手：在编辑器加一个自定义面板（如显示当前 draw call 数，配合 2.3 的统计接口）。

### 2.10 资源 resource（★）
- 关键文件：`resource/asset_manager/*`、`config_manager/*`、`res_type/*`
- 核心概念：资源加载 / 缓存 / 引用计数、资源类型（mesh / texture / material / level）、异步加载。
- 动手：跟一遍「一个 mesh 从磁盘到渲染」的完整路径。

---

## 3. 阶段三：综合改造与优化（跨模块，推 GitHub）

把前面学到的东西用起来，做能「看到效果 / 用数据证明」的改造。每个都开分支、提交、推送。

**渲染相关（你最熟，先上手）**
- [ ] 方向光阴影 Shadow Map（加 shadow pass）
- [ ] 抗锯齿升级 FXAA → SMAA / TAA
- [ ] 视锥剔除（减少 draw call，配合统计 HUD 看数字）
- [ ] 后处理合并（tone_mapping + color_grading + fxaa 合一，省带宽）
- [ ] SSAO / Bloom（扩战后处理）

**物理相关（补盲 + 实用）**
- [ ] 碰撞体调试可视化（debugdraw 画线框）
- [ ] 碰撞事件系统（触发音效 / 逻辑）
- [ ] 物理性能剖析（刚体数 vs 耗时）

**跨模块**
- [ ] 用反射给所有组件加统一调试面板
- [ ] 自定义 Component 端到端跑通（2.2 的延伸）

---

## 4. 推荐节奏
- 第 1 周：阶段一（整体概览）+ 2.1 / 2.2（core + ECS）。
- 第 2–4 周：2.3 渲染（重点，最细）。
- 第 5–6 周：2.4 物理（补盲）+ 2.5 / 2.6 动画 / 角色。
- 第 7 周起：2.7–2.10 输入 / 粒子 / UI / 资源 快速过 + 进入阶段三改造。
