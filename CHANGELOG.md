# Prism 更新日志

> **项目代号**: Prism (寓意：将混沌的配置像棱镜一样解析得清晰分明)
> **最后更新**: 2026-01-05

---

## v1.0.0-alpha (2026-01-05)

### 新功能 - 首页功能完善 🏠

#### 首页数据统计卡片
- **统计卡片区域**: 横向布局显示三项核心数据
  - 总项目数：显示当前打开的项目数量
  - 配置文件：显示所有项目的配置文件总数
  - 运行进程：显示当前运行中的进程数量
- **卡片样式**: 左侧图标 + 右侧数值/标签，支持深色/浅色主题

#### 最近打开的项目列表
- **项目卡片**: 显示项目名称、路径、最后打开时间、配置文件数量
- **智能排序**: 按最后打开时间降序排列，最多显示 5 个
- **点击导航**: 点击项目卡片直接跳转到对应项目的配置页面
- **空状态提示**: 无项目时显示"暂无项目，请先导入或创建项目"

#### 快捷操作面板
- **打开配置按钮**: 快速跳转到当前激活项目的配置页面
- **智能提示**: 未选择项目时提示用户先打开项目

#### 布局设计
- **紧凑布局**: 统计卡片在顶部，内容区域左右分栏（70%/30%）
- **主题自适应**: 所有组件支持 Dark/Light 主题切换
- **面板样式**: 统一的圆角边框、背景色、悬停效果

### Bug 修复 - 首页数据加载 🔧

#### 问题描述
- 启动软件后首页统计数据显示为 0，最近项目列表为空
- 只有当项目数据变化后才会刷新显示

#### 根本原因
- `setupConnections()` 在构造函数中调用时，`window()` 返回 nullptr
- 信号连接失败，无法接收 `projectDataChanged` 信号

#### 解决方案
- 使用 `showEvent` 延迟初始化
- 首次显示时建立信号连接并刷新数据
- 添加 `_isInitialized` 标志防止重复初始化

### 技术实现

#### MainWindow 数据接口
- `getAllProjects()`: 获取所有项目列表
- `getOpenedProjectsCount()`: 获取打开的项目数量
- `getTotalConfigFilesCount()`: 获取配置文件总数
- `getRunningProcessCount()`: 获取运行中进程数量
- `getProjectConfigPageKey()`: 获取项目配置页面的导航 Key
- `projectDataChanged` 信号: 项目数据变化时通知首页刷新

#### HomePage 组件
- `createStatCard()`: 创建统计卡片（横向布局）
- `createProjectCard()`: 创建项目卡片
- `refreshData()`: 刷新首页数据
- `showEvent()`: 延迟初始化，首次显示时建立连接

#### 修改文件
- `src/ui/MainWindow.h/cpp` - 添加数据接口和 projectDataChanged 信号
- `src/ui/ProcessPage.h/cpp` - 添加 `getRunningProcessCount()` 方法
- `src/ui/HomePage.h/cpp` - 完整重写，实现统计卡片、最近项目、快捷操作

---

## v1.0.0-alpha (2026-01-03)

### 新功能 - 多进程监控（表格式布局）🎯

#### 重构进程监控页面
- **表格式布局**: 参考 Supervisor/PM2 设计，使用 `ElaTableView` + `QStandardItemModel`
- **弹窗式编辑**: 使用 `ElaDialog` 创建/编辑进程配置
- **进程表格**: 显示状态、名称、程序路径、操作按钮
- **独立日志 Tab**: 每个进程有独立的日志页面 + "全部"汇总页
- **配置持久化**: 按项目保存多个进程配置（JSON 格式）

#### 控制台窗口选项
- **显示控制台开关**: 在进程配置中添加"显示控制台"选项（`ElaToggleSwitch`）
- **Windows 支持**: 使用 `CREATE_NEW_CONSOLE` 标志显示独立控制台窗口
- **智能提示**: 开启后提示"日志将不会在此处显示"
- **配置保存**: `showConsole` 字段持久化到配置文件

#### UI 组件升级
- **ElaTableView**: 替换 `QTableWidget`，原生主题支持，性能更好
- **ElaDialog**: 替换 `QDialog`，自动适配深色/浅色主题
- **移除自定义样式**: 删除约 60 行表格样式代码，使用 Ela 原生样式

#### 技术实现
- **ProcessConfig**: 添加 `showConsole` 字段
- **ProcessRunner**: 支持 `showConsole` 参数，Windows 平台使用 `setCreateProcessArgumentsModifier()`
- **ProcessEditDialog**: 继承 `ElaDialog`，添加控制台开关
- **ProcessPage**: 使用 `ElaTableView` + `QStandardItemModel`，移除 `updateTableStyle()`

#### 修改文件
- `src/ui/ProcessEditDialog.h/cpp` - 添加控制台开关，继承 `ElaDialog`
- `src/ui/ProcessPage.h/cpp` - 重构为表格式布局，使用 `ElaTableView`
- `src/core/ProcessRunner.h/cpp` - 添加 `showConsole` 参数支持

---

## v1.0.0-alpha (2025-12-17)

### Bug 修复 - 配置快照页面布局 🔧
- 修复页面标题显示为 "Page_0" 的问题
- 修复快照卡片布局错乱（内容挤在一行）
- 修复详情对话框布局问题
- 修复空状态提示不显示的问题
- 使用正确的 `ElaScrollPage` 布局方式（`addCentralWidget()`）
- 区分 `ElaScrollArea`（滚动容器）和 `ElaScrollPageArea`（样式区域）

---

## v1.0.0-alpha (2025-12-16)

### 新功能 - 配置快照系统 🎯
- **SnapshotManager**: 创建/应用/删除/重命名快照
- **CreateSnapshotDialog**: 选择文件、输入名称描述、格式验证
- **SnapshotPage**: 卡片式布局显示快照列表
- **快照存储**: `.prism/snapshots/` 目录，JSON 元数据管理
- **智能验证**: 创建前验证配置文件格式
- **主窗口集成**: 每个项目添加"配置快照"页面

---

## v1.0.0-alpha (2025-12-15)

### 新功能 - 进程监控功能
- **ProcessRunner 集成**: 自动编码转换（GBK → UTF-8）
- **进程停止优化**: 异步超时处理，修复卡顿和误报
- **配置持久化**: 保存程序路径和参数到 QSettings
- **浏览程序功能**: 文件选择对话框
- **日志输出统一**: 8 个日志级别，主题自适应

### Bug 修复 - 配置文件持久化
- 修复删除/移除/重命名后重启恢复的问题
- 添加信号监听，同步更新 QSettings

### 优化 - UI 对话框
- 删除确认对话框优化（隐藏中间按钮）
- 重命名对话框使用 `ElaContentDialog`

---

## v1.0.0-alpha (2025-12-14)

### 新功能
- **配置文件右键菜单**: 打开/显示/备份/重命名/移除/删除
- **编辑模式切换优化**: 自动保存提醒
- **组件模式添加配置项**: 支持多种类型，智能验证

### Bug 修复
- 修复 INI 逗号分隔值丢失、中文乱码
- 修复 JSON 数组内对象键顺序打乱
- 修复 YAML 保存格式问题

### 架构优化
- **Parser 重构**: 统一分隔符为 `.`，使用 ParserFactory
- **配置文件管理**: 优先扫描 `config` 子目录
- **日志系统**: 全面集成，8 个日志级别

---

## 技术栈

- **开发语言**: C++17
- **UI 框架**: Qt 5.14.2 + ElaWidgetTools (Fluent Design)
- **构建系统**: CMake 3.15+
- **编译器**: MSVC 2022 (Windows) / GCC (Linux)
- **配置解析**: yaml-cpp, Qt JSON, 自定义 IniParser

---

**维护者**: 国腾天创 Prism 团队
