# Pilot引擎 0.0.8 版本发布说明
✨ 大家好！Pilot引擎自4月4日发布以来，我们很高兴得到很多开发者朋友们的关注，非常感谢社区开发者们的贡献！
在整合了开发者社区贡献的更改和我们内部开发更改之后，在此我们激动地发布**Pilot引擎0.0.8版本**！

## 版本信息
- 发布版本：v0.0.8
- 发布时间：2022年9月12日

## 更新内容
### 新特性
#### 引擎
- 基于GPU的粒子系统 #345
- F键切换自由相机/角色跟随相机 #345
- 增加了复合哈希函数 #281

#### 编辑器
- 将bool型变量反射为复选框控件 #335

### 重构
- 移除了渲染层的GLM数学库，统一为内置数学库 #345

### 优化
- 减少不必要的内存拷贝，提高渲染性能 #296

### 修复的bug
#### 编辑器
- 修复了Components Details面板修改变换无法移动模型的问题 #251

#### 引擎
- 修复了Forward渲染中FXAA Pass指针使用错误的的问题 #320
- 修复了Apple Sillicon上的编译问题 #326
- 修复了第一人称LookAt矩阵构建bug #334
- 修复了渲染结果无法跟随ImGUI窗口移动的bug #345

## What's Changed
* fix compilation by @OlorinMedas in https://github.com/BoomingTech/Piccolo/pull/280
* Add hash_combine by @ShenMian in https://github.com/BoomingTech/Piccolo/pull/281
* Fix the misalignment of rotation in <TransformComponent> by @Killyice in https://github.com/BoomingTech/Piccolo/pull/283
* Reduce unnecessary copies of memory data to improve rendering performance by @kwbm in https://github.com/BoomingTech/Piccolo/pull/296
* fix "setTile" typo by @chaihahaha in https://github.com/BoomingTech/Piccolo/pull/303
* feature: Add checkbox for bool field in editor ui by @CRAFTSTARCN in https://github.com/BoomingTech/Piccolo/pull/335
* render_pipeline.cpp中forwardRender()函数，创建fxaa_pass时错误使用了m_tone_mapping_pass.get() by @sorvon in https://github.com/BoomingTech/Piccolo/pull/320
* fix: Support build on Apple Sillicon. by @snakeeye in https://github.com/BoomingTech/Piccolo/pull/326
* fix first person camera lookatMat by @KVM-Explorer in https://github.com/BoomingTech/Piccolo/pull/334
* 0.0.8 by @BoomingTechDev in https://github.com/BoomingTech/Piccolo/pull/345

## New Contributors
* @Killyice made their first contribution in https://github.com/BoomingTech/Piccolo/pull/283
* @chaihahaha made their first contribution in https://github.com/BoomingTech/Piccolo/pull/303
* @sorvon made their first contribution in https://github.com/BoomingTech/Piccolo/pull/320
* @snakeeye made their first contribution in https://github.com/BoomingTech/Piccolo/pull/326
* @KVM-Explorer made their first contribution in https://github.com/BoomingTech/Piccolo/pull/334

**Full Changelog**: https://github.com/BoomingTech/Piccolo/compare/v0.0.7...v0.0.8

# Pilot引擎 0.0.7 版本发布说明
✨ 大家好！Pilot引擎自4月4日发布以来，我们很高兴得到很多开发者朋友们的关注，非常感谢社区开发者们的贡献！
在整合了开发者社区贡献的更改和我们内部开发更改之后，在此我们激动地发布**Pilot引擎0.0.7版本**！

## 版本信息
- 发布版本：v0.0.7
- 发布时间：2022年6月29日

## 更新内容
### 新特性
#### 编辑器
- 允许关闭面板 #242

#### 引擎
- 运行时不再依赖硬编码的路径，bin目录可以整体打包拷贝到其他目录运行 #262
- 可以在render.global.json配置文件中通过配置开关FXAA #218
- Motor系统加入跳跃功能 #276
- Character Controller改用JoltPhysics的isOverlap实现 #276

### 修复的bug
#### 编辑器
- 修复了Components Details面板修改变换无法移动模型的问题 #251
#### 引擎
- 修复了macOS下Vulkan验证层报错的bug #249
- 修复了macOS某些版本下std::aligned_alloc编译报错的问题 #276
- 修复了面板反射函数调用错误 #276

## What's Changed
* Fix namespace typo by @kwbm in https://github.com/BoomingTech/Piccolo/pull/230
* Editor GUI improved. by @sselecirPyM in https://github.com/BoomingTech/Piccolo/pull/242
* set transform component of selected object dirty by @OlorinMedas in https://github.com/BoomingTech/Piccolo/pull/251
* FIX bug : macos validation layer error message by @rocketman123456 in https://github.com/BoomingTech/Piccolo/pull/249
* Relative root path by @OlorinMedas in https://github.com/BoomingTech/Piccolo/pull/262
* add "enable_fxaa" in rendering.global.json to enable/disable fxaa pass by @jiangdunchun in https://github.com/BoomingTech/Piccolo/pull/218
* MetaParser, RHI, some render bugfixes by @BoomingTechDev in https://github.com/BoomingTech/Piccolo/pull/276

## New Contributors
* @sselecirPyM made their first contribution in https://github.com/BoomingTech/Piccolo/pull/242

**Full Changelog**: https://github.com/BoomingTech/Piccolo/compare/v0.0.6...v0.0.7

# Pilot引擎 0.0.6 版本发布说明
✨ 大家好！Pilot引擎自4月4日发布以来，我们很高兴得到很多开发者朋友们的关注，非常感谢社区开发者们的贡献！
在整合了开发者社区贡献的更改和我们内部开发更改之后，在此我们激动地发布**Pilot引擎0.0.6版本**！

## 版本信息
- 发布版本：v0.0.6
- 发布时间：2022年6月1日

## 更新内容
### 新特性
- 接入了JoltPhysic物理引擎 #213 #214 #215

### 重构
- 渲染数据交换和渲染相机移至渲染层 #206 
- 命名规范 #220

### 优化
#### 运行时：
- 优化了打开物理调试窗口时掉帧的问题 #222
- 裁剪JoltPhyiscs资产 #222

### 修复的bug

#### 引擎运行时：
- 修复了重新加载关卡时渲染系统中的bug #205
- 修复了色彩分级贴图的Mip层级中的bug #211
- 修复了删除物体和退出时崩溃的bug #219
- 修复了macOS下Vulkan验证层报错的bug #209
- 修复了其他一些bug #210

## What's Changed
* Fix render system problem when reloading current level by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/205
* Code Cleanup: move swap_data and render_camera into render system by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/206
* add JoltPhysics library by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/213
* Fix color grading texture's mipmap level bug by @kwbm in https://github.com/BoomingTech/Pilot/pull/211
* add jolt physics build dir by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/214
* small bug fix by @renjingneng in https://github.com/BoomingTech/Pilot/pull/210
* Integrate Jolt Physics Engine by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/215
* Fix the crash while deleting object and exiting by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/219
* refactor: naming convention by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/220
* FIX bug : fix macos validation layer error bug by @rocketman123456 in https://github.com/BoomingTech/Pilot/pull/209
* Jolt Physics Optimization: DebugRenderer by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/222

## New Contributors
* @renjingneng made their first contribution in https://github.com/BoomingTech/Pilot/pull/210
* @rocketman123456 made their first contribution in https://github.com/BoomingTech/Pilot/pull/209

**Full Changelog**: https://github.com/BoomingTech/Pilot/compare/v0.0.5...v0.0.6

# Pilot引擎 0.0.5 版本发布说明
✨ 大家好！Pilot引擎自4月4日发布以来，我们很高兴得到很多开发者朋友们的关注，非常感谢社区开发者们的贡献！
在整合了开发者社区贡献的更改和我们内部开发更改之后，在此我们激动地发布**Pilot引擎0.0.5版本**！

## 版本信息
- 发布版本：v0.0.5
- 发布时间：2022年5月25日

## 更新内容
### 新特性
- FXAA #185

### 重构
- 重构了核心框架 #177 #179 #200 #201
  - 使用 `WorldManager` 和 `Level` 中使用智能指针管理资源
  - 使用 `ObjectIDAllocator` 分配游戏对象ID
  - 使用 `GObjectID` 替代 `size_t` 作为游戏对象ID
  - 加载世界时改为仅加载默认关卡
  - 使用角色游戏对象名称替代游戏对象序号作为当前角色游戏对象的索引
  - 将组件资产数据合并至游戏对象资产数据中
  - 将组件编辑器模式下是否需要tick的属性移至编辑器全局属性管理
  - 先加载游戏对象实例的组件资产数据，再加载游戏对象定义中的组件资产数据
  - TransformComponent改为普通的实例化组件
  - 为组件加载新增资产加载后处理函数 `postLoadResource`
  - 单例类重构为全局上下文
- 重构了渲染系统 #198 #199
  - 新增了交换数据结构以管理逻辑与渲染数据交换
  - 将原渲染系统整合为RHI、RenderScene、RenderResource、RenderPipeline
  - 分离出Vulkan相关的逻辑
  - 将编辑器UI与渲染逻辑解耦
- 重构了编辑器架构 #181 #194
  - 使用 `EditorSceneManager` 管理编辑器模式下的渲染场景
  - 使用 `EditorGlobalContext` 管理编辑器模式下的全局系统
  - 抽象了鼠标事件（选中物体、高亮选中轴、相机速度调整等逻辑）
  - 抽象了键盘输入事件（相机移动，删除等逻辑）
  - 编辑器模式输入事件从引擎中分离到了工具层
  - EditorMode及GameMode的判定切换逻辑更改
- 统一了组件定义类的命名风格 #156
- 资产根据规范重新组织 #158
- AssetManager统一使用资产相对路径加载/保存资产 #160
- 代码根据Wiki推荐方式重构 #161
- 优化了元数据反射指针深拷贝方式 #162

### 优化
#### 编辑器：
- 优化了编辑器模式相机在高分辨率屏幕下的手感 #154

### 编译环境：
- 优化了Linux下如果可以使用Ninja默认使用Ninja作为构建系统 #172 #174
- 增加了编译数据库以优化开发环境 #180

#### 代码： 
- 代码优化 #155 #157 #163 #176 #183 #189 #190 #193 #195 #201

### 修复的bug

#### 引擎运行时：
- 修复了subpass依赖关系处理的错误 #153

## What's Changed
* Fix Rendering Bug by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/153
* [Optimize] Fix editor camera is too sensitive for 4k monitor/screen by @boooooommmmmm in https://github.com/BoomingTech/Pilot/pull/154
* Remove unnecessary code + Fix config_manager.h by @ShenMian in https://github.com/BoomingTech/Pilot/pull/155
* Unify component definition class naming style by @OlorinMedas in https://github.com/BoomingTech/Pilot/pull/156
* Refactor META usage by @OlorinMedas in https://github.com/BoomingTech/Pilot/pull/157
* Asset Components Reorganization by @OlorinMedas in https://github.com/BoomingTech/Pilot/pull/158
* AssetManager Load Save Url Refactor by @OlorinMedas in https://github.com/BoomingTech/Pilot/pull/160
* Refactor for Documentation 1 by @OlorinMedas in https://github.com/BoomingTech/Pilot/pull/161
* Refactor Deep Copy Macro and Format by @OlorinMedas in https://github.com/BoomingTech/Pilot/pull/162
* Fix variable naming: m_enable_debug_untils_label -> m_enable_debug_utils_label by @yuruofeifei in https://github.com/BoomingTech/Pilot/pull/163
* Allow using ninja on default if ninja is found on Linux platform by @VitalyAnkh in https://github.com/BoomingTech/Pilot/pull/172
* Add `-j$(nproc)` to the build command by @VitalyAnkh in https://github.com/BoomingTech/Pilot/pull/174
* fix typo by @LJHG in https://github.com/BoomingTech/Pilot/pull/176
* Refactor Level System by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/177
* Refactor Editor Framework by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/181
* Refactor component system by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/179
* Add the building script for compile db by @huandzh in https://github.com/BoomingTech/Pilot/pull/180
* Fix typo: filed -> field by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/183
* Remove unnecessary code by @ShenMian in https://github.com/BoomingTech/Pilot/pull/189
* Fix level.cpp by @ShenMian in https://github.com/BoomingTech/Pilot/pull/190
* Passing std::string by reference by @ShenMian in https://github.com/BoomingTech/Pilot/pull/193
* Fix compile error on some VC compiler (unmatch function declaration) by @kwbm in https://github.com/BoomingTech/Pilot/pull/195
* Refactor Editor: ui layer and input layer by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/194
* Implement FXAA algorithm by @jiangdunchun in https://github.com/BoomingTech/Pilot/pull/185
* Refactor Rendering System by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/198
* Fix RHI crash, rename m_rhi to be m_vulkan_rhi by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/199
* Refactor singleton with global context by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/200
* clean-code: naming convention by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/201
* Fix the crash while exit by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/203

## New Contributors
* @yuruofeifei made their first contribution in https://github.com/BoomingTech/Pilot/pull/163
* @VitalyAnkh made their first contribution in https://github.com/BoomingTech/Pilot/pull/172
* @LJHG made their first contribution in https://github.com/BoomingTech/Pilot/pull/176
* @huandzh made their first contribution in https://github.com/BoomingTech/Pilot/pull/180
* @jiangdunchun made their first contribution in https://github.com/BoomingTech/Pilot/pull/185

**Full Changelog**: https://github.com/BoomingTech/Pilot/compare/v0.0.3...v0.0.5

# Pilot引擎 0.0.3 版本发布说明
✨ 大家好！Pilot引擎发布三周以来，我们很高兴得到很多开发者朋友们的关注，非常感谢社区开发者们的贡献！
在整合了开发者社区贡献的更改和我们内部开发更改之后，在此我们激动地发布**Pilot引擎0.0.3版本**！

## 版本信息
- 发布版本：v0.0.3
- 发布时间：2022年4月26日

## 更新内容

### 新特性
- 支持延迟渲染管线 #146
- 相机角色跟随融合 #143
- 运动停走跑状态切换并引入加速度 #144
- 全局渲染资源配置分离 #145
- 按住鼠标右键滑动滚轮可以调整相机移动速度 #140

### 优化

#### 编辑器：
- ComponentDetail面板中旋转改为欧拉角表示 #142
- 优化了Editor模式下相机控制 #114
- 修复了Editor模式下指针闪烁的问题 #116
- 修复了Editor面板按钮覆盖的问题 #118

#### 代码： 
- 一些代码优化 #110 #115 #121 #125 #134

### 修复的bug

#### 引擎运行时：
- 修复了pick pass中图像布局信息转换会导致未定义行为的错误 #111
- 修复了Vulkan初始化时AMD集显与NVIDIA独显竞争的问题 #137


## What's Changed
* some code optimization by @Wlain in https://github.com/BoomingTech/Pilot/pull/110
* Fix image layout transition in 'pick' pass by @AirGuanZ in https://github.com/BoomingTech/Pilot/pull/111
* Add virtual destructor for class Pilot::Controller by @micro123 in https://github.com/BoomingTech/Pilot/pull/115
* [Updated] A better editor camera controlling by @Chaphlagical in https://github.com/BoomingTech/Pilot/pull/114
* [Fixed] Cursor twinkling in editor mode by @Chaphlagical in https://github.com/BoomingTech/Pilot/pull/116
* Fix overlapped button, ImGui::Indent should also consider DPI scale by @iaomw in https://github.com/BoomingTech/Pilot/pull/118
* Use STL algorithm by @ShenMian in https://github.com/BoomingTech/Pilot/pull/121
* Add Visual Studio configuration file to .gitignore by @ShenMian in https://github.com/BoomingTech/Pilot/pull/123
* Remove unnecessary code by @ShenMian in https://github.com/BoomingTech/Pilot/pull/125
* Set the startup project PilotEditor in Visual Studio by @hebohang in https://github.com/BoomingTech/Pilot/pull/132
* Use STL algorithm by @ShenMian in https://github.com/BoomingTech/Pilot/pull/134
* Disable AMD switchable graphics device to avoid enumerating error by @kwbm in https://github.com/BoomingTech/Pilot/pull/137
* [enhancement] using euler angle to display rotation in editor by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/142
* camera character position blend by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/143
* Fix motor state by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/144
* refactor global render resource config by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/145
* Implement deferred rendering pipeline by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/146
* update lut file path by @BoomingTechDev in https://github.com/BoomingTech/Pilot/pull/148
* can adjust camera move speed by holding right mouse button and scroll by @boooooommmmmm in https://github.com/BoomingTech/Pilot/pull/140

## New Contributors
* @AirGuanZ made their first contribution in https://github.com/BoomingTech/Pilot/pull/111
* @micro123 made their first contribution in https://github.com/BoomingTech/Pilot/pull/115
* @Chaphlagical made their first contribution in https://github.com/BoomingTech/Pilot/pull/114
* @hebohang made their first contribution in https://github.com/BoomingTech/Pilot/pull/132
* @BoomingTechDev made their first contribution in https://github.com/BoomingTech/Pilot/pull/143
* @boooooommmmmm made their first contribution in https://github.com/BoomingTech/Pilot/pull/140

**Full Changelog**: https://github.com/BoomingTech/Pilot/compare/v0.0.2...v0.0.3

# Pilot引擎 0.0.2 版本发布说明
✨ 大家好！Pilot引擎发布一周以来，我们很高兴得到很多开发者朋友们的关注，非常感谢社区开发者们的贡献！
在整合了开发者社区贡献的更改和我们内部开发更改之后，在此我们激动地发布**Pilot引擎0.0.2版本**！

## 版本信息
- 发布版本：v0.0.2
- 发布时间：2022年4月12日

## 更新内容

### 优化

#### 编辑器：
- 优化了文件内容面板刷新文件树的性能 #67 
- 优化了组件细节面板变换组件坐标轴对应颜色显示

#### 代码： 
- 优化了一些接口设计

### 修复的bug

#### 引擎运行时：
- 修复了重新加载当前关卡导致Pilot引擎崩溃的问题 #3 
- 修复了碰撞体未跟随所附物体的transform进行变换的问题 #17 
- 修复了组件细节面板里的变换参数无法通过拖动或者修改字符进行修改的问题 #76 
- 修复了第三人称相机未使用资源中配置的参数的问题
- 修复了相机视锥体边缘物体阴影计算的问题
- 修复了粗糙度为0时高光计算错误的问题
- 修复了Vulkan优先选择独显运行Pilot的问题 #14 #32 
- 修复了高分屏视口大小位置不正确的问题 #12 
- 修复了无法找到合适显示设备时抛出错误异常的问题 
- 修复了M1 macOS版本崩溃的问题 #27 

#### 编辑器：

- 修复了macOS下高DPI字体显示模糊的问题 #4 #84 

#### 代码： 

- 修正了单例类的实现方式

#### 编译：

- 修复了Windows下使用MSVC并行编译配置错误的问题 #9 
- 修复了Ubuntu下apt install执行权限问题
- 修复了Linux下meta parser相关可执行二进制文件执行权限引起的编译失败的问题 #10 #16 
- 移除了Linux生成脚本中的macOS命令
- 修复了不完备包含的头文件可能导致编译错误的问题
- 修复了M1版本macOS编译问题 #1 #11 #56 
- 修复了Shader目录不存在可能导致编译失败的问题 #33 
- 修复了Linux下对tbb的依赖可能导致编译失败的问题 #95 

## What's Changed
* Fix CMakeLists.txt by @ShenMian in https://github.com/BoomingTech/Pilot/pull/18
* Fix typo on the course name by @KSkun in https://github.com/BoomingTech/Pilot/pull/26
* Correct apt command by @mieotoha in https://github.com/BoomingTech/Pilot/pull/24
* Fix: score gpu and rank before check physical device suitability to find the best gpu for rending by @CRAFTSTARCN in https://github.com/BoomingTech/Pilot/pull/29
* fix compile warning by @wlbksy in https://github.com/BoomingTech/Pilot/pull/36
* Exclude 3rdparty/ from root clang-format config by @cadenji in https://github.com/BoomingTech/Pilot/pull/38
* Fix typos by @ambiguoustexture in https://github.com/BoomingTech/Pilot/pull/37
* Fix: format-security by @ambiguoustexture in https://github.com/BoomingTech/Pilot/pull/39
* Add high dpi support by @kwbm in https://github.com/BoomingTech/Pilot/pull/41
* [Linux] Fix meta parser RPATH by @archibate in https://github.com/BoomingTech/Pilot/pull/20
* Several minor fixes by @Altair-Alpha in https://github.com/BoomingTech/Pilot/pull/44
* remove const before POD in function arguments by @wlbksy in https://github.com/BoomingTech/Pilot/pull/47
* Remove the macOS command from linux build script by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/49
* Fix std::runtime_error message by @ShenMian in https://github.com/BoomingTech/Pilot/pull/51
* add github workflows by @YuqiaoZhang in https://github.com/BoomingTech/Pilot/pull/54
* Fix header file missing (which would cause compile error on Fedora OS) by @kwbm in https://github.com/BoomingTech/Pilot/pull/46
* Fix build macos by @YuqiaoZhang in https://github.com/BoomingTech/Pilot/pull/53
* rename MacOS to masOS by @BoomingTech-YuqiaoZhang in https://github.com/BoomingTech/Pilot/pull/55
* Fix build linux by @YuqiaoZhang in https://github.com/BoomingTech/Pilot/pull/52
* Update build_linux.yml by @ShenMian in https://github.com/BoomingTech/Pilot/pull/59
* Fix Apple M1 Crash by @BoomingTech-YuqiaoZhang in https://github.com/BoomingTech/Pilot/pull/65
* Optimize third person camera settings and calculation by @kwbm in https://github.com/BoomingTech/Pilot/pull/60
* create a CMakeLists.txt file at the root dir by @hybcloud in https://github.com/BoomingTech/Pilot/pull/63
* feature: add macOS script too by @wlbksy in https://github.com/BoomingTech/Pilot/pull/71
* Refactor: Use STL by @wlbksy in https://github.com/BoomingTech/Pilot/pull/73
* Update editor_ui by @ShenMian in https://github.com/BoomingTech/Pilot/pull/74
* Add ISSUE_TEMPLATE by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/75
* Update directional_light.cpp by @ShenMian in https://github.com/BoomingTech/Pilot/pull/78
* Un-capitalize build script for macOS by @ambiguoustexture in https://github.com/BoomingTech/Pilot/pull/77
* Update editor_ui by @ShenMian in https://github.com/BoomingTech/Pilot/pull/80
* Fix third-person camera rotation by @OlorinMedas in https://github.com/BoomingTech/Pilot/pull/90
* DPI awareness font scaling by @iaomw in https://github.com/BoomingTech/Pilot/pull/85
* Add build windows workflow by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/93
* feat: Beautify transform component UI by @ImGili in https://github.com/BoomingTech/Pilot/pull/94
* Fix: format-security by @ambiguoustexture in https://github.com/BoomingTech/Pilot/pull/96
* Fix directional light camera calculation by @YuqiaoZhang in https://github.com/BoomingTech/Pilot/pull/81
* More fixes by @Altair-Alpha in https://github.com/BoomingTech/Pilot/pull/98
* Fix compile error caused by spv dir not exist. by @yjhgithub in https://github.com/BoomingTech/Pilot/pull/102
* update gitignore for clion by @Wlain in https://github.com/BoomingTech/Pilot/pull/103
* Update editor_ui.cpp by @ShenMian in https://github.com/BoomingTech/Pilot/pull/104
* Fix singleton classes by @ShenMian in https://github.com/BoomingTech/Pilot/pull/100
* Remove tbb lib dependence by @hyv1001 in https://github.com/BoomingTech/Pilot/pull/105
* fix: mesh.frag no spec color when roughness == 0 by @deepkolos in https://github.com/BoomingTech/Pilot/pull/107
* Pilot internal development release 20220412 by @Ol6rin in https://github.com/BoomingTech/Pilot/pull/108

## New Contributors
* @ShenMian made their first contribution in https://github.com/BoomingTech/Pilot/pull/18
* @KSkun made their first contribution in https://github.com/BoomingTech/Pilot/pull/26
* @mieotoha made their first contribution in https://github.com/BoomingTech/Pilot/pull/24
* @CRAFTSTARCN made their first contribution in https://github.com/BoomingTech/Pilot/pull/29
* @wlbksy made their first contribution in https://github.com/BoomingTech/Pilot/pull/36
* @cadenji made their first contribution in https://github.com/BoomingTech/Pilot/pull/38
* @ambiguoustexture made their first contribution in https://github.com/BoomingTech/Pilot/pull/37
* @kwbm made their first contribution in https://github.com/BoomingTech/Pilot/pull/41
* @archibate made their first contribution in https://github.com/BoomingTech/Pilot/pull/20
* @Altair-Alpha made their first contribution in https://github.com/BoomingTech/Pilot/pull/44
* @hyv1001 made their first contribution in https://github.com/BoomingTech/Pilot/pull/49
* @YuqiaoZhang made their first contribution in https://github.com/BoomingTech/Pilot/pull/54
* @BoomingTech-YuqiaoZhang made their first contribution in https://github.com/BoomingTech/Pilot/pull/55
* @hybcloud made their first contribution in https://github.com/BoomingTech/Pilot/pull/63
* @OlorinMedas made their first contribution in https://github.com/BoomingTech/Pilot/pull/90
* @iaomw made their first contribution in https://github.com/BoomingTech/Pilot/pull/85
* @ImGili made their first contribution in https://github.com/BoomingTech/Pilot/pull/94
* @yjhgithub made their first contribution in https://github.com/BoomingTech/Pilot/pull/102
* @Wlain made their first contribution in https://github.com/BoomingTech/Pilot/pull/103
* @deepkolos made their first contribution in https://github.com/BoomingTech/Pilot/pull/107
* @Ol6rin made their first contribution in https://github.com/BoomingTech/Pilot/pull/108

**Full Changelog**: https://github.com/BoomingTech/Pilot/compare/v0.0.1...v0.0.2

# Pilot引擎 0.0.1 版本发布说明
First Release, Enjoy Coding!

版本信息
发布版本：v0.0.1
发布时间：2022年4月4日
