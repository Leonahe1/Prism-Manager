# Prism 开发路线图

> **项目代号**: Prism (寓意：将混沌的配置像棱镜一样解析得清晰分明)
> **最后更新**: 2025-12-01
> **当前版本**: v1.0.0-alpha

---

## 📋 项目概述

**Prism** 是一款现代化的仿真配置集成环境，专为雷达算法工程师设计。

### 核心定位
- 参考 **Postman** 的环境管理理念
- 参考 **Supervisor** 的进程监控架构
- 提供统一的配置文件管理和仿真程序运行监控

### 技术栈
- **开发语言**: C++17
- **UI 框架**: Qt 5.14.2 + ElaWidgetTools (Fluent Design)
- **构建系统**: CMake 3.15+
- **编译器**: MSVC 2022 (Windows) / GCC (Linux)
- **配置解析**: yaml-cpp, Qt JSON

---

## ✅ 最新完成 (2025-12-01)

### **1. 配置文件管理优化**

#### 1.1 配置文件扫描路径优化

**问题**：项目导入时扫描配置文件的位置不合理，直接扫描根目录导致配置文件混乱。

**解决方案**：
- ✅ 优先扫描项目根目录下的 `config` 子目录
- ✅ 如果 `config` 目录不存在，则回退到扫描根目录
- ✅ 添加详细的调试日志，显示扫描路径和找到的文件数量

**代码位置**：`MainWindow.cpp:355-372`

**实现代码**：
```cpp
// 扫描配置文件（优先扫描 config 子目录，如果不存在则扫描根目录）
QStringList nameFilters = {"*.ini", "*.json", "*.yaml", "*.yml", "*.conf"};
QDir configDir(projectDir.absoluteFilePath("config"));

QFileInfoList fileInfoList;
if (configDir.exists()) {
    fileInfoList = configDir.entryInfoList(nameFilters, QDir::Files);
} else {
    fileInfoList = projectDir.entryInfoList(nameFilters, QDir::Files);
}
```

---

#### 1.2 完善"添加配置文件"功能

**问题**：之前的 `addConfigFile` 功能只是打开文件选择对话框，但没有真正将文件添加到项目的配置文件列表中。

**解决方案**：
- ✅ 在 `ConfigPage` 中添加项目根路径成员变量 `_currentProjectRootPath`
- ✅ 在 `ConfigPage` 中添加配置文件列表成员变量 `_currentConfigFiles`
- ✅ 添加 `setProjectRootPath()` 方法设置项目根路径
- ✅ 添加 `configFileAdded` 信号通知 MainWindow 更新项目配置
- ✅ 文件选择对话框默认打开项目的 `config` 目录（如果存在）
- ✅ 检查文件是否已存在，避免重复添加
- ✅ 添加文件后自动刷新配置文件列表
- ✅ 在 MainWindow 中监听信号并保存项目配置
- ✅ 添加成功操作的日志记录

**代码位置**：
- `ConfigPage.h:78, 102, 222-223` - 新增方法和成员变量
- `ConfigPage.cpp:671-675, 949-1000` - 实现逻辑
- `MainWindow.cpp:379, 395-403` - 设置根路径和信号连接

**技术亮点**：
- ✅ 智能路径检测：优先使用 `config` 目录
- ✅ 重复检测：避免添加已存在的文件
- ✅ 信号-槽机制：ConfigPage 与 MainWindow 解耦
- ✅ 自动持久化：添加文件后自动保存到项目配置
- ✅ 日志追踪：完整记录文件添加操作

---

#### 1.3 修复配置文件列表持久化 🎯 **核心修复**

**问题**：配置文件列表没有被持久化保存，第二次打开项目时，手动添加的配置文件会丢失。

**根本原因**：
- `saveProjectsToSettings()` 只保存了项目路径，**未保存 `configFiles` 列表**
- `loadProjectsFromSettings()` 重新调用 `addProject()` 重新扫描目录
- 手动添加的文件（不在 `config` 目录）无法被重新扫描到

**解决方案**：

**1. 改进持久化存储格式**
- ✅ 为每个项目保存完整的 `ProjectInfo` 信息
- ✅ 包括 `id`, `name`, `rootPath`, `configFiles`, `lastOpened`
- ✅ 使用分组结构 `project_0`, `project_1` ... 存储多个项目

**代码位置**：`MainWindow.cpp:726-754`

```cpp
void MainWindow::saveProjectsToSettings()
{
    QSettings settings("GTTC", "Prism");
    settings.beginGroup("Projects");
    settings.remove("");  // 清除旧数据

    settings.setValue("count", _openedProjects.size());

    int index = 0;
    for (const auto& project : _openedProjects) {
        settings.beginGroup(QString("project_%1").arg(index));
        settings.setValue("id", project.id);
        settings.setValue("name", project.name);
        settings.setValue("rootPath", project.rootPath);
        settings.setValue("configFiles", project.configFiles);  // ⭐ 关键：保存配置文件列表
        settings.setValue("lastOpened", project.lastOpened);
        settings.endGroup();
        index++;
    }
}
```

**2. 改进加载逻辑**
- ✅ 直接恢复保存的 `ProjectInfo`，不再调用 `addProject()`
- ✅ 如果配置文件列表为空（兼容旧版本），则重新扫描
- ✅ 恢复项目时重新连接 `configFileAdded` 信号

**代码位置**：`MainWindow.cpp:756-863`

```cpp
void MainWindow::loadProjectsFromSettings()
{
    // 读取项目数量
    int projectCount = settings.value("count", 0).toInt();

    for (int i = 0; i < projectCount; ++i) {
        // 恢复 ProjectInfo
        info.configFiles = settings.value("configFiles").toStringList();  // ⭐ 恢复配置文件列表

        // 兼容旧版本：如果列表为空，重新扫描
        if (info.configFiles.isEmpty()) {
            // ... 重新扫描逻辑 ...
        }

        // 创建页面并连接信号
        connect(configPage, &ConfigPage::configFileAdded, this, [this, projectId](const QString& filePath) {
            _openedProjects[projectId].configFiles.append(filePath);
            saveProjectsToSettings();  // ⭐ 自动保存
        });
    }
}
```

**技术亮点**：
- ✅ 完整持久化：所有项目信息都被保存
- ✅ 增量更新：添加文件后自动保存
- ✅ 向后兼容：空列表时自动重新扫描
- ✅ 信号驱动：配置变更自动触发保存
- ✅ 数据完整性：项目恢复时配置文件列表完全一致

---

### **2. Parser 架构优化**

#### 2.1 统一 Parser 嵌套键分隔符

**问题**：三个 Parser 的嵌套键分隔符不一致，导致难以统一处理。
- `YamlParser`: 使用 `"."` （如 `"server.host"`）
- `JsonParser`: 使用 `"."` （如 `"database.port"`）
- `IniParser`: 使用 `"/"` （如 `"section/key"`） ❌ 不一致

**解决方案**：
- ✅ 修改 `IniParser` 使用 `"."` 作为分隔符
- ✅ 统一所有 Parser 的输出格式
- ✅ 简化 ConfigPage 的解析逻辑

**代码位置**：`src/core/IniParser.cpp:59-75`

```cpp
void IniParser::readAllKeys(QSettings& settings, QVariantMap& map, const QString& prefix) {
    QStringList keys = settings.childKeys();
    for (const QString& key : keys) {
        QString fullKey = prefix.isEmpty() ? key : prefix + "." + key;  // ✅ 改为 "."
        map[fullKey] = settings.value(key);
    }

    QStringList groups = settings.childGroups();
    for (const QString& group : groups) {
        settings.beginGroup(group);
        QString newPrefix = prefix.isEmpty() ? group : prefix + "." + group;  // ✅ 改为 "."
        readAllKeys(settings, map, newPrefix);
        settings.endGroup();
    }
}
```

**影响范围**：
- ✅ INI 格式：`[Database]` 下的 `host=localhost`
  - 旧格式：`Database/host`
  - 新格式：`Database.host`
- ✅ 不影响文件内容，只影响内存表示
- ✅ 为后续重构 ConfigPage 使用 ParserFactory 做准备

---

#### 2.2 重构 ConfigPage 使用 ParserFactory 🎯 **核心重构**

**问题**：ConfigPage 重复实现了解析逻辑，与 `core/Parser` 架构重复。
- ❌ `parseIniContent()`, `parseYamlContent()`, `parseJsonContent()` 重复实现
- ❌ `buildIniForm()`, `buildYamlForm()`, `buildJsonForm()` 逻辑相似
- ❌ `collectIniFormToSource()`, `collectYamlFormToSource()`, `collectJsonFormToSource()` 重复代码
- ❌ 没有利用 ParserFactory 的自动格式检测和保存功能

**重构方案**：

**1. 新增统一方法**

| 旧方法 (已删除) | 新方法 | 说明 |
|----------------|--------|------|
| `parseIniContent`<br>`parseYamlContent`<br>`parseJsonContent` | `parseConfigFile(filePath)` | 使用 ParserFactory 统一解析 |
| `buildIniForm`<br>`buildYamlForm`<br>`buildJsonForm` | `buildForm(configData)` | 统一的表单构建逻辑 |
| `collectIniFormToSource`<br>`collectYamlFormToSource`<br>`collectJsonFormToSource` | `collectFormAndSave()` | 使用 ParserFactory 保存 |

**2. 新增类型推断方法**

```cpp
ConfigItem variantToConfigItem(const QVariant& variant)
```
- 从 `QVariant` 推断数据类型
- 支持 Bool, Int, Double, String, Array
- 智能检测字符串形式的布尔值和数字

**3. 简化的调用流程**

**旧流程** (按格式分支):
```cpp
syncSourceToForm() {
    if (format == "ini")    buildIniForm();
    if (format == "yaml")   buildYamlForm();
    if (format == "json")   buildJsonForm();
}

syncFormToSource() {
    if (format == "ini")    collectIniFormToSource();
    if (format == "yaml")   collectYamlFormToSource();
    if (format == "json")   collectJsonFormToSource();
}
```

**新流程** (统一处理):
```cpp
syncSourceToForm() {
    auto configData = parseConfigFile(_currentFilePath);  // 自动检测格式
    buildForm(configData);  // 统一构建
}

syncFormToSource() {
    collectFormAndSave();  // 自动保存
}
```

**4. 核心实现**

**parseConfigFile** - 使用 ParserFactory (ConfigPage.cpp:1113-1139):
```cpp
QMap<QString, ConfigItem> ConfigPage::parseConfigFile(const QString& filePath)
{
    ParserFactory& factory = ParserFactory::instance();
    QVariantMap variantMap = factory.parse(filePath);  // 自动检测格式

    QMap<QString, ConfigItem> result;
    for (auto it = variantMap.constBegin(); it != variantMap.constEnd(); ++it) {
        result[it.key()] = variantToConfigItem(it.value());  // 类型推断
    }
    return result;
}
```

**collectFormAndSave** - 使用 ParserFactory 保存 (ConfigPage.cpp:1331-1403):
```cpp
void ConfigPage::collectFormAndSave()
{
    // 从控件收集数据到 QVariantMap
    QVariantMap configData;
    for (auto it = _formWidgetMap.begin(); it != _formWidgetMap.end(); ++it) {
        // 从 lineEdit/spinBox/toggleSwitch 获取值
        configData[it.key()] = value;
    }

    // 使用 ParserFactory 保存
    ParserFactory& factory = ParserFactory::instance();
    ConfigFormat format = factory.detectFormat(_currentFilePath);
    bool success = factory.save(_currentFilePath, configData, format);

    // 保存后重新读取到源码编辑器
    if (success) {
        // 重新加载文件内容
    }
}
```

**5. 技术亮点**

- ✅ **消除重复代码**：删除 9 个格式特定方法，替换为 3 个统一方法
- ✅ **自动格式检测**：利用 ParserFactory 的格式检测能力
- ✅ **类型安全**：统一的 `ConfigItem` 类型系统
- ✅ **易于扩展**：添加新格式只需在 core/Parser 添加，ConfigPage 无需修改
- ✅ **双向同步**：源码 ↔ 表单完全自动化

**6. 代码统计**

| 指标 | 重构前 | 重构后 | 减少 |
|-----|--------|--------|------|
| 方法数量 | 12 个 | 6 个 | -50% |
| 代码行数 | ~1800 行 | ~900 行 | -50% |
| 格式分支 | 每个操作 3 个分支 | 0 分支 | -100% |

**7. 影响的文件**

**ConfigPage.h**:
- 新增: `variantToConfigItem()`, `parseConfigFile()`, `buildForm()`, `collectFormAndSave()`
- 删除: 9 个格式特定方法

**ConfigPage.cpp**:
- 新增实现: 4 个统一方法 (~350 行)
- 删除实现: 9 个格式特定方法 (~1200 行)
- 修改: `syncSourceToForm()`, `syncFormToSource()`, `saveCurrentConfig()`

**8. 向后兼容性**

- ✅ 用户数据完全兼容（文件格式未变）
- ✅ UI 界面完全一致（表单布局未变）
- ✅ 功能完全相同（只是实现方式改变）

---

### **日志系统集成 - 全面日志追踪**

#### 完成内容
在项目管理和配置文件操作中全面集成日志输出功能，实现了所有关键操作的日志追踪。

#### 集成位置

| 功能模块 | 操作 | 日志级别 | 示例 |
|---------|------|---------|------|
| **项目管理** | 项目导入 | `SUCCESS` | `项目已导入: "RadarSim" (D:\Projects\RadarSim)` |
| | 项目移除 | `WARNING` | `项目已从列表移除: "RadarSim"` |
| | 项目关闭 | `INFO` | `已关闭项目: "RadarSim"` |
| | 项目切换 | `INFO` | `已切换到项目: "RadarSim"` |
| **配置文件** | 打开文件 | `INFO` | `打开配置文件: config.yaml (格式: YAML)` |
| | 保存文件 | `SUCCESS` | `配置文件已保存: config.yaml` |
| | 验证成功 | `SUCCESS` | `YAML 验证通过: config.yaml (共 45 个节点)` |
| | 验证失败 | `ERROR` | `JSON 验证失败 (config.json): Unexpected token` |
| | 文件错误 | `ERROR` | `无法打开配置文件: D:\config.yaml` |
| **应用启动** | 初始化 | `SUCCESS` | `Prism 初始化完成` |
| | 项目恢复 | `INFO` | `等待用户操作...` |

#### 技术实现

**MainWindow.cpp** (src/ui/MainWindow.cpp)
- 调整初始化顺序，确保日志窗口先于项目加载创建
- 在 `addProject()` 中添加项目导入成功日志
- 在 `removeProject()` 中添加项目移除警告日志
- 在 `deactivateProject()` 中添加项目关闭信息日志
- 在 `setActiveProject()` 中添加项目切换信息日志

**ConfigPage.cpp** (src/ui/ConfigPage.cpp)
- 添加 `#include "MainWindow.h"` 引用
- 通过 `qobject_cast<Prism::MainWindow*>(window())` 获取主窗口引用
- 在 `openConfigFile()` 中添加打开成功/失败日志
- 在 `saveCurrentConfig()` 中添加保存成功/失败日志
- 在 `validateConfig()` 中为所有格式（INI/YAML/JSON）添加验证结果日志

#### 代码示例

```cpp
// 项目导入日志
if (!silent) {
    appendLog("SUCCESS", QString("项目已导入: \"%1\" (%2)")
        .arg(info.name, projectPath));
}

// 配置文件打开日志
if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
    mainWin->appendLog("INFO", QString("打开配置文件: %1 (格式: %2)")
        .arg(fileName, _currentFormat.toUpper()));
}

// 验证失败日志
if (auto* mainWin = qobject_cast<Prism::MainWindow*>(window())) {
    mainWin->appendLog("ERROR", QString("YAML 验证失败 (%1): %2")
        .arg(fileName, QString::fromStdString(e.what())));
}
```

#### 技术亮点
- ✅ 完整覆盖所有项目管理操作
- ✅ 完整覆盖所有配置文件操作
- ✅ 统一的日志格式和级别使用
- ✅ 详细的错误信息记录（包含文件名和错误描述）
- ✅ 成功操作提供完整上下文（如格式类型、节点数等）

---

## ✅ 之前完成 (2025-11-30)

### 1. **配置文件表单模式 - 智能类型映射系统**

#### 完成内容
实现了 **INI、YAML、JSON** 三种格式的智能表单模式，根据值的类型自动映射到不同的 UI 组件：

| 数据类型 | UI 组件 | 示例 |
|---------|---------|------|
| `Boolean` | `ElaToggleSwitch` 开关 | true/false → ●━━━○ |
| `Integer` | `ElaSpinBox` 整数微调框 | 8080 → [8080 ▲▼] |
| `Double` | `ElaDoubleSpinBox` 浮点微调框 | 3.14 → [3.14 ▲▼] |
| `String` | `ElaLineEdit` 文本输入框 | localhost → [localhost___] |
| `Array` | `ElaLineEdit` 文本输入框 | [1,2,3] → [[1,2,3]____] |

#### 技术亮点
- ✅ 自动类型检测（布尔/整数/浮点/字符串）
- ✅ 双向数据同步（表单 ↔ 源码）
- ✅ 主题自适应（Dark/Light 模式）
- ✅ 实时修改监听，自动标记为修改状态

#### 代码位置
- 头文件: `src/ui/ConfigPage.h` (新增 `ConfigValueType` 枚举和 `ConfigItem` 结构体)
- 实现文件: `src/ui/ConfigPage.cpp`
  - `parseYamlContent()` - YAML 解析与类型检测
  - `buildYamlForm()` - YAML 表单构建
  - `collectYamlFormToSource()` - YAML 数据收集
  - `parseIniContent()` - INI 解析与类型检测
  - `buildIniForm()` - INI 表单构建
  - `collectIniFormToSource()` - INI 数据收集
  - `parseJsonContent()` - JSON 解析与类型检测
  - `buildJsonForm()` - JSON 表单构建
  - `collectJsonFormToSource()` - JSON 数据收集

---

### 2. **日志系统 - 主题自适应与分级输出**

#### 完成内容
实现了完整的日志系统，支持 **8 个日志级别** 和 **主题自适应**。

#### 日志级别
| 级别 | 用途 | Dark 颜色 | Light 颜色 | 示例 |
|------|------|----------|-----------|------|
| `INFO` | 一般信息 | 蓝色 #61AFEF | 蓝色 #0078D4 | `[INFO] Prism 管理器已初始化` |
| `SUCCESS` | 成功操作 | 绿色 #98C379 | 绿色 #107C10 | `[SUCCESS] 配置文件已保存` |
| `WARNING` | 警告信息 | 黄色 #E5C07B | 橙色 #F7630C | `[WARNING] 配置验证失败` |
| `ERROR` | 错误信息 | 红色 #E06C75 | 红色 #D13438 | `[ERROR] 无法打开文件` |
| `DEBUG` | 调试信息 | 紫色 #C678DD | 紫色 #8764B8 | `[DEBUG] 主题已切换: Dark` |
| `PROCESS` | 进程状态 | 青色 #56B6C2 | 青色 #00979C | `[PROCESS] 启动进程: sim.exe` |
| `STDOUT` | 标准输出 | 灰白 #ABB2BF | 深灰 #323130 | `[STDOUT] Simulation started` |
| `STDERR` | 错误输出 | 红色 #E06C75 | 红色 #D13438 | `[STDERR] Warning: Low memory` |

#### 技术亮点
- ✅ 自动时间戳（精确到秒）
- ✅ HTML 富文本彩色输出
- ✅ 主题切换时自动更新背景和文字颜色
- ✅ 自动滚动到底部
- ✅ Light 主题对比度问题已修复（白色背景 + 深色文字）

#### 代码位置
- 头文件: `src/ui/MainWindow.h`
  - `appendLog(const QString& level, const QString& message)` 公共接口
  - `onThemeChanged(ElaThemeType::ThemeMode mode)` 主题响应槽函数
  - `_logTextEdit` 成员变量
- 实现文件: `src/ui/MainWindow.cpp`
  - `appendLog()` - 日志追加逻辑
  - `onThemeChanged()` - 主题切换逻辑

---

## 🎯 后续开发方向

### 阶段 1: 核心功能完善 (优先级: 🔴 高)

#### 1.1 日志系统集成 ✅ **已完成 (2025-12-01)**
- [x] 在项目导入/移除操作中添加日志
  - `appendLog("SUCCESS", "项目 \"XXX\" 已导入")`
  - `appendLog("WARNING", "项目 \"XXX\" 已移除")`
- [x] 在配置文件操作中添加日志
  - 打开文件: `appendLog("INFO", "打开配置文件: xxx.yaml")`
  - 保存文件: `appendLog("SUCCESS", "配置文件已保存")`
  - 验证失败: `appendLog("ERROR", "YAML 验证失败: 第 12 行语法错误")`
- [ ] 在进程管理中添加日志 ⚠️ **待完成（进程监控功能尚未实现）**
  - `appendLog("PROCESS", "启动进程: radar_sim.exe")`
  - `appendLog("STDOUT", processOutput)` - 实时输出
  - `appendLog("STDERR", errorOutput)` - 错误输出
  - `appendLog("PROCESS", "进程已停止 (退出码: 0)")`

**实现总结**:
- ✅ MainWindow 项目管理操作日志完整集成
- ✅ ConfigPage 配置文件操作日志完整集成
- ✅ 通过 `qobject_cast<MainWindow*>(window())` 实现跨页面日志访问
- ⏳ 进程日志待 ProcessRunner 实现后集成

---

#### 1.2 进程监控功能增强
- [ ] 实时捕获进程输出
  - 使用 `ProcessRunner` 的信号槽机制
  - 将 `stdout` 输出到日志窗口（`STDOUT` 级别）
  - 将 `stderr` 输出到日志窗口（`STDERR` 级别）
- [ ] 进程状态指示灯优化
  - 运行中：绿色闪烁
  - 已停止：灰色
  - 错误退出：红色
- [ ] 进程日志过滤功能
  - 添加过滤器：只显示 `ERROR`/`WARNING`
  - 添加搜索框：关键字高亮

**技术要点**:
```cpp
// 在 ProcessRunner 中连接信号
connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
    QString output = m_process->readAllStandardOutput();
    emit outputReceived(output);  // 发送信号
});

// 在 ProcessPage 中接收并转发到日志
connect(processRunner, &ProcessRunner::outputReceived, this, [mainWin](const QString& output) {
    mainWin->appendLog("STDOUT", output);
});
```

---

#### 1.3 配置文件语法高亮
- [ ] 源码模式下添加语法高亮
  - INI 格式高亮（section/key/value）
  - YAML 格式高亮（使用 QSyntaxHighlighter）
  - JSON 格式高亮
- [ ] 行号显示
- [ ] 错误行标记（验证失败时高亮错误行）

**技术要点**:
```cpp
class YamlSyntaxHighlighter : public QSyntaxHighlighter {
    // 参考 Qt 文档实现语法高亮
};
```

---

### 阶段 2: 用户体验优化 (优先级: 🟡 中)

#### 2.1 配置文件模板系统
- [ ] 内置常用配置模板
  - 雷达仿真默认配置
  - 网络配置模板
  - 算法参数模板
- [ ] 一键创建新配置文件
  - 选择模板 → 填写参数 → 生成文件

---

#### 2.2 配置文件对比功能
- [ ] 对比两个配置文件的差异
  - 使用 diff 算法
  - 高亮显示差异部分
- [ ] 配置历史版本管理
  - 自动备份修改前的配置
  - 支持恢复到历史版本

---

#### 2.3 快捷键系统
- [ ] 全局快捷键
  - `Ctrl+S` - 保存当前配置
  - `Ctrl+W` - 关闭当前项目
  - `Ctrl+Shift+P` - 快速打开项目
  - `Ctrl+Shift+F` - 全局搜索配置项
- [ ] 编辑器快捷键
  - `Ctrl+F` - 查找
  - `Ctrl+H` - 替换
  - `Ctrl+/` - 注释/取消注释

---

### 阶段 3: 高级功能 (优先级: 🟢 低)

#### 3.1 局域网配置同步
- [ ] 设计 `IDataSource` 接口（预留）
  - 本地文件源: `FileDataSource`
  - 网络源: `NetworkDataSource`
- [ ] TCP/UDP 配置广播
  - 主机模式：广播配置变更
  - 从机模式：自动同步配置
- [ ] 配置冲突解决机制

---

#### 3.2 插件系统
- [ ] 自定义配置解析器插件
  - 允许用户扩展支持的配置格式
- [ ] 自定义主题插件
  - 允许用户自定义配色方案

---

#### 3.3 数据可视化
- [ ] 配置项关系图谱
  - 可视化配置依赖关系
- [ ] 进程资源监控图表
  - CPU 使用率
  - 内存使用率
  - 运行时长

---

## 🔧 技术债务与优化

### 代码质量
- [ ] 添加单元测试
  - `ConfigParser` 测试
  - `ProcessRunner` 测试
- [ ] 添加注释文档
  - 为关键方法添加 Doxygen 注释
- [ ] 性能优化
  - 大型配置文件加载优化
  - 日志窗口性能优化（限制最大行数）

### 错误处理
- [ ] 完善异常处理
  - 文件读写失败处理
  - 进程启动失败处理
- [ ] 用户友好的错误提示
  - 替换 `QMessageBox` 为 `ElaMessageBar`

---

## 📝 开发规范

### 代码风格
- **命名规范**: 遵循 Qt 风格（驼峰命名）
- **文件组织**: 头文件 `.h` + 实现文件 `.cpp`
- **注释**: 使用中文注释，关键接口使用 Doxygen 格式

### Git 提交规范
- `feat: 新功能描述`
- `fix: 修复问题描述`
- `refactor: 重构描述`
- `docs: 文档更新`

### 构建测试
- 每次提交前执行 `cmake --build . --config Debug` 确保编译通过
- 手动测试核心功能（项目导入、配置保存、进程启动）

---

## 🎨 设计理念

### UI/UX 原则
1. **简洁至上**: 避免过度设计，保持界面整洁
2. **一致性**: 统一的配色、图标、交互方式
3. **响应式**: 操作后立即反馈（MessageBar/日志）
4. **主题自适应**: 所有组件必须支持 Dark/Light 主题

### 架构原则
1. **分离关注点**: UI 层、业务逻辑层、数据层分离
2. **可扩展性**: 使用接口/抽象类预留扩展点
3. **模块化**: 每个功能模块独立，降低耦合

---

## 🚀 里程碑

### v1.0.0-alpha (当前)
- ✅ 基础项目管理
- ✅ 配置文件表单模式（INI/YAML/JSON）
- ✅ 日志系统（主题自适应）
- ⏳ 进程监控功能

### v1.1.0-beta (目标: 2周后)
- 日志系统集成完成
- 进程实时输出捕获
- 配置文件语法高亮

### v1.5.0-rc (目标: 1个月后)
- 配置模板系统
- 配置对比功能
- 快捷键系统

### v2.0.0-stable (目标: 2个月后)
- 局域网配置同步
- 插件系统
- 数据可视化

---

## 📞 联系方式

**项目维护者**: 国腾天创 Prism 团队
**技术支持**: 通过 Issues 反馈问题
**文档**: 参考 `CLAUDE.md` 了解项目架构

---

## 📚 参考资源

- [Qt 5.14 文档](https://doc.qt.io/qt-5.14/)
- [ElaWidgetTools](https://github.com/Liniyous/ElaWidgetTools)
- [yaml-cpp 文档](https://github.com/jbeder/yaml-cpp)
- [CMake 文档](https://cmake.org/documentation/)

---

**最后更新**: 2025-11-30
**文档版本**: v1.0
