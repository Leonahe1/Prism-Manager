# Prism 更新日志

> **项目代号**: Prism (寓意：将混沌的配置像棱镜一样解析得清晰分明)
> **最后更新**: 2026-01-16

---

## v1.0.0-alpha (2026-01-16)

### 新功能 - 设置页面 ⚙️

#### 设置管理类 (AppSettings)
- **单例模式**: 统一管理所有应用配置项
- **QSettings 持久化**: 存储在 `HKCU\Software\GTTC\Prism`
- **信号通知**: 设置变更时发出信号，支持实时响应
- **重置功能**: 一键恢复所有配置为默认值

#### 设置页面 (SettingsPage)
- **外观设置**:
  - 主题模式（浅色/深色/跟随系统）- 即时生效
  - 界面语言（预留，暂仅支持中文）
- **编辑器设置**:
  - 代码字体（Consolas/Courier New/Monaco 等等宽字体）
  - 字体大小（8-24）
  - Tab 宽度（2-8）
  - 显示行号（开/关）
- **日志设置**:
  - 最大日志行数（1000-100000）
  - 时间戳格式（HH:mm:ss / HH:mm:ss.zzz / yyyy-MM-dd HH:mm:ss）
  - 默认显示级别（多选：INFO/SUCCESS/WARNING/ERROR/DEBUG/STDOUT/STDERR）
- **通用设置**:
  - 启动时打开上次项目
  - 最近项目数量（3-10）
  - 记住窗口位置
  - 自动保存间隔（0-60 分钟，0=禁用）

#### UI 设计
- **卡片式布局**: 四大配置分类，每个分类独立卡片
- **主题自适应**: 卡片边框、背景色随主题切换
- **底部按钮**: "重置为默认" + "应用"

### UI 优化 - 主工具栏精简 🔧

- **移除无用按钮**: 删除工具栏中未实现功能的"运行"和"停止"按钮
- **保留核心功能**: 仅保留"打开项目"按钮
- **界面更简洁**: 减少用户困惑，提升界面清爽度

### Bug 修复 - 配置编辑模式切换 🔧

#### 问题描述
- 第一次打开配置文件后，未做任何修改
- 从组件模式切换到源码模式时，误报"组件模式有未保存的修改"

#### 根本原因
- `openConfigFile` 中调用 `syncSourceToForm()` 后，表单控件初始化时触发了信号
- 导致 `_isModified` 被错误设置为 `true`

#### 解决方案
- 在 `syncSourceToForm()` 完成后，显式重置 `_isModified = false`
- 确保表单初始化不会被误认为是用户修改

### 修改文件
- `src/core/AppSettings.h/cpp` - 新建设置管理单例类
- `src/ui/SettingsPage.h/cpp` - 新建设置页面
- `src/ui/MainWindow.h` - 添加 SettingsPage 类型声明
- `src/ui/MainWindow.cpp` - 集成 SettingsPage，移除工具栏无用按钮
- `src/ui/ConfigPage.cpp` - 修复模式切换误报 bug
- `src/CMakeLists.txt` - 添加新文件到构建系统

---

## v1.0.0-alpha (2026-01-15)

### 新功能 - 配置文件语法高亮 🎨

#### 语法高亮器
- **SyntaxHighlighter 基类**: 可扩展的语法高亮框架，支持主题切换
- **IniHighlighter**: INI 格式语法高亮
  - Section 高亮: `[section]` - 蓝色/青色
  - 键值对: key=value - 默认色/绿色
  - 注释高亮: `;comment` / `#comment` - 灰色
- **JsonHighlighter**: JSON 格式语法高亮
  - 键名: `"key":` - 蓝色
  - 字符串: `"value"` - 绿色
  - 数字: `123` - 橙色
  - 布尔值: `true/false` - 紫色
  - Null: `null` - 红色
- **YamlHighlighter**: YAML 格式语法高亮
  - 键名: `key:` - 蓝色
  - 字符串/数字/布尔值 - 绿色/橙色/紫色
  - 列表标记: `-` - 黄色
  - 注释: `#comment` - 灰色

#### 代码编辑器组件
- **CodeEditor**: 带行号显示的代码编辑器
  - 行号区域: 自动计算宽度，随滚动更新
  - 当前行高亮: 浅色背景突出显示
  - 主题自适应: 支持 Dark/Light 模式切换

#### 技术实现
- 基于 `QSyntaxHighlighter` 实现，使用正则表达式匹配语法元素
- 颜色方案区分深色/浅色主题，自动响应 `ElaTheme::themeModeChanged` 信号
- `CodeEditor` 继承自 `QPlainTextEdit`，内嵌 `LineNumberArea` 子组件

### 新功能 - 日志增强功能 📋

#### 日志工具栏 (LogToolBar)
- **级别过滤**: 7 个复选框，支持按日志级别过滤
  - INFO / SUCCESS / WARNING / ERROR / DEBUG / STDOUT / STDERR
- **关键字搜索**: 实时搜索日志内容，匹配文本黄色高亮
- **导出功能**: 导出为 UTF-8 编码的 .txt 文件
- **清空功能**: 一键清空当前日志

#### 日志过滤与搜索
- **日志缓存**: 保存所有日志条目，支持重新过滤显示
- **实时过滤**: 切换过滤条件后立即刷新日志显示
- **搜索高亮**: 使用 `QTextCharFormat` 设置黄色背景高亮匹配文本
- **滚动定位**: 搜索后自动滚动到第一个匹配位置

#### 日志导出
- **文件格式**: 纯文本 (.txt)，UTF-8 编码
- **内容格式**: `[时间戳] [级别] [进程ID] 消息`
- **文件命名**: `{项目名}_logs_{日期时间}.txt`

#### 日志行数限制
- **最大行数**: 10000 行（可配置）
- **自动清理**: 超过限制时自动删除最旧的日志行
- **性能保护**: 防止大量日志导致内存溢出

#### 修改文件
- `src/ui/syntax/SyntaxHighlighter.h/cpp` - 新建语法高亮基类
- `src/ui/syntax/IniHighlighter.h/cpp` - 新建 INI 高亮器
- `src/ui/syntax/JsonHighlighter.h/cpp` - 新建 JSON 高亮器
- `src/ui/syntax/YamlHighlighter.h/cpp` - 新建 YAML 高亮器
- `src/ui/components/CodeEditor.h/cpp` - 新建代码编辑器组件
- `src/ui/components/LogToolBar.h/cpp` - 新建日志工具栏组件
- `src/ui/ConfigPage.h/cpp` - 集成语法高亮器
- `src/ui/ProcessPage.h/cpp` - 集成日志增强功能
- `src/CMakeLists.txt` - 添加新文件到构建系统

---

## v1.0.0-alpha (2026-01-06)

### 新功能 - 进程状态指示灯 🚦

#### 可视化状态指示
- **圆形指示灯组件**: 替换文本状态，直观显示进程状态
- **5种状态颜色**:
  - 🟢 运行中（绿色闪烁）
  - 🟡 启动中（黄色闪烁）
  - ⚪ 已停止（灰色静态）
  - 🔴 错误退出（红色静态）
  - 🟠 强制终止（橙色静态）
- **闪烁动画**: 1秒周期，使用 QTimer 实现
- **状态提示**: 鼠标悬停显示状态名称

#### 技术实现
- **ProcessStatusIndicator**: 自定义 QWidget 组件，支持闪烁和主题适配
- **ProcessRunner**: 新增 `Starting` 状态枚举
- **ProcessPage**: 使用容器布局实现指示灯居中显示

#### 修改文件
- `src/ui/ProcessStatusIndicator.h/cpp` - 新建状态指示灯组件
- `src/core/ProcessRunner.h` - 添加 Starting 状态
- `src/ui/ProcessPage.h/cpp` - 集成指示灯组件

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
