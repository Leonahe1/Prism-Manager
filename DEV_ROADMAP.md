# Prism 开发路线图

> **项目代号**: Prism (寓意：将混沌的配置像棱镜一样解析得清晰分明)
> **最后更新**: 2025-12-15
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
- **配置解析**: yaml-cpp, Qt JSON, 自定义 IniParser

---

## ✅ 最新完成 (2025-12-15)

### **进程监控功能完整实现** 🎯

#### 1. ProcessRunner 集成
- ✅ 将 ProcessRunner 完全集成到 ProcessPage
- ✅ 替换原生 QProcess 为封装的 ProcessRunner
- ✅ 自动编码转换（Windows GBK → UTF-8）
- ✅ 完善的状态管理（5 种状态）
- ✅ 实时日志输出（STDOUT/STDERR）

#### 2. 进程停止优化
- ✅ 修复停止按钮卡顿问题（移除阻塞调用）
- ✅ 修复"进程异常终止"误报（区分用户停止和真正崩溃）
- ✅ 异步超时处理（3秒后自动强制杀死）
- ✅ 友好的用户提示（停止/退出/崩溃分别提示）

#### 3. 进程配置持久化
- ✅ 保存程序路径和命令参数到 QSettings
- ✅ 按项目分别保存配置
- ✅ 自动加载上次配置
- ✅ 输入框变化时自动保存

#### 4. 浏览程序功能
- ✅ 添加"浏览..."按钮
- ✅ 文件选择对话框（Windows: .exe/.bat/.cmd）
- ✅ 自动填充到输入框
- ✅ 记录日志

#### 5. 日志输出统一
- ✅ ProcessPage 日志样式与 MainWindow 一致
- ✅ 8 个日志级别（INFO/SUCCESS/WARNING/ERROR/DEBUG/PROCESS/STDOUT/STDERR）
- ✅ 主题自适应（Dark/Light 模式）
- ✅ HTML 格式化彩色输出
- ✅ 自动滚动到底部

#### 技术亮点
- ✅ **非阻塞设计**: 所有操作不阻塞 UI 线程
- ✅ **智能状态管理**: 5 种状态（NotStarted/Running/Finished/Error/Killed）
- ✅ **自动编码转换**: ProcessRunner 内部处理 GBK → UTF-8
- ✅ **按行输出**: 自动分割换行符，过滤空行
- ✅ **持久化记忆**: 每个项目独立保存进程配置

---

### **配置文件持久化修复** (2025-12-15)

#### 问题描述
使用右键菜单的删除、移除、重命名功能后，当前界面正确更新，但软件重启后恢复到初始状态。

#### 根本原因
MainWindow 没有监听 ConfigPage 的 `configFileRemoved`、`configFileDeleted`、`configFileRenamed` 信号，导致项目配置文件列表没有同步更新到 QSettings。

#### 解决方案
1. ✅ 添加 `configFileRenamed` 信号（ConfigPage.h）
2. ✅ 在重命名操作中发射信号（ConfigPage.cpp）
3. ✅ 在 MainWindow 中连接所有信号：
   - `configFileRemoved` → 从列表移除 + 保存
   - `configFileDeleted` → 从列表移除 + 保存
   - `configFileRenamed` → 更新列表路径 + 保存
4. ✅ 在 `addProject()` 和 `loadProjectsFromSettings()` 中都连接信号

#### 修复效果
- ✅ 删除文件后重启，文件不再出现
- ✅ 移除文件后重启，文件不再出现
- ✅ 重命名文件后重启，新文件名正确保存

---

### **UI 对话框优化** (2025-12-15)

#### 1. 删除确认对话框
- ✅ 隐藏中间按钮（通过 `findChildren<ElaPushButton*>()` 手动隐藏）
- ✅ 只显示"取消"和"删除"两个按钮
- ✅ 界面更简洁

#### 2. 重命名对话框
- ✅ 替换 QInputDialog 为 ElaContentDialog
- ✅ 使用 ElaLineEdit 作为输入框
- ✅ 使用 ElaText 作为提示文本
- ✅ 支持回车键快速确认
- ✅ 自动选中文件名，方便修改
- ✅ Fluent Design 风格，与项目整体风格统一

---

## ✅ 之前完成 (2025-12-13 ~ 2025-12-14)

### **JSON 数组内对象键顺序保持修复**
- ✅ 修复数组内对象键顺序被打乱的问题
- ✅ 新增 `extractArrayElementKeyOrders()` 方法
- ✅ 支持多层嵌套数组（如 `data.items[0].nested[1].value`）
- ✅ 递归处理数组内对象的嵌套对象和嵌套数组

### **配置文件保存优化 - 增量更新策略**
- ✅ JSON 格式：保持键顺序、空对象、null 值
- ✅ YAML 格式：保持原始结构
- ✅ 非侵入式更新：只修改用户编辑的字段
- ✅ 容错机制：解析失败时自动回退到重建方式

### **配置文件管理优化**
- ✅ 优先扫描项目 `config` 子目录
- ✅ 完善配置文件列表持久化
- ✅ 添加配置文件右键菜单（打开/显示/备份/重命名/移除/删除）
- ✅ 组件模式添加新配置项功能

### **Parser 架构重构**
- ✅ 统一三种 Parser 的嵌套键分隔符为 `.`
- ✅ 重构 ConfigPage 使用 ParserFactory
- ✅ 删除 9 个格式特定方法，替换为 3 个统一方法
- ✅ 代码量减少 50%

### **日志系统**
- ✅ 8 个日志级别（INFO/SUCCESS/WARNING/ERROR/DEBUG/PROCESS/STDOUT/STDERR）
- ✅ 主题自适应（Dark/Light 模式）
- ✅ HTML 富文本彩色输出
- ✅ 全面集成到项目管理和配置文件操作

---

## 🎯 后续开发方向

### 阶段 1: 核心功能完善 (优先级: 🔴 高)

#### 1.1 进程监控增强 ⚠️ **部分完成**
- [x] 实时捕获进程输出 ✅ **已完成**
  - 使用 ProcessRunner 的信号槽机制
  - 将 stdout 输出到日志窗口（STDOUT 级别）
  - 将 stderr 输出到日志窗口（STDERR 级别）
- [x] 进程配置持久化 ✅ **已完成**
  - 保存程序路径和命令参数
  - 按项目分别保存
  - 自动加载上次配置
- [ ] 进程状态指示灯优化 ⏳ **待完成**
  - 运行中：绿色闪烁
  - 已停止：灰色
  - 错误退出：红色
- [ ] 进程日志过滤功能 ⏳ **待完成**
  - 添加过滤器：只显示 ERROR/WARNING
  - 添加搜索框：关键字高亮
- [ ] 进程日志导出功能 ⏳ **待完成**
  - 导出为 .txt 文件
  - 包含时间戳和日志级别
- [ ] 进程日志行数限制 ⏳ **待完成**
  - 限制最大行数（如 10000 行）
  - 自动删除旧日志，防止内存溢出

**技术要点**:
```cpp
// 进程状态指示灯
ElaProgressBar* _statusBar;  // 使用进度条模拟状态灯
_statusBar->setTextVisible(false);
_statusBar->setMaximum(0);  // 无限循环模式（运行中）

// 日志过滤
enum class LogFilter {
    All,        // 显示全部
    StdoutOnly, // 仅标准输出
    StderrOnly, // 仅错误输出
    ErrorsOnly  // 仅错误和警告
};

// 日志导出
void ProcessPage::exportLog() {
    QString fileName = QFileDialog::getSaveFileName(...);
    QFile file(fileName);
    file.write(_logTextEdit->toPlainText().toUtf8());
}
```

---

#### 1.2 配置文件语法高亮 ⏳ **待完成**
- [ ] 源码模式下添加语法高亮
  - INI 格式高亮（section/key/value）
  - YAML 格式高亮（使用 QSyntaxHighlighter）
  - JSON 格式高亮
- [ ] 行号显示
- [ ] 错误行标记（验证失败时高亮错误行）

**技术要点**:
```cpp
class YamlSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit YamlSyntaxHighlighter(QTextDocument* parent = nullptr);
protected:
    void highlightBlock(const QString& text) override;
private:
    QVector<HighlightingRule> highlightingRules;
};

// 在 ConfigPage 中使用
_sourceTextEdit = new ElaPlainTextEdit(this);
_yamlHighlighter = new YamlSyntaxHighlighter(_sourceTextEdit->document());
```

---

#### 1.3 配置文件模板系统 ⏳ **待完成**
- [ ] 内置常用配置模板
  - 雷达仿真默认配置
  - 网络配置模板
  - 算法参数模板
- [ ] 一键创建新配置文件
  - 选择模板 → 填写参数 → 生成文件
- [ ] 模板管理界面
  - 查看所有模板
  - 编辑自定义模板
  - 导入/导出模板

**技术要点**:
```cpp
// 模板结构
struct ConfigTemplate {
    QString name;           // 模板名称
    QString description;    // 模板描述
    ConfigFormat format;    // 格式（INI/JSON/YAML）
    QString content;        // 模板内容
    QStringList variables;  // 变量列表（如 ${PROJECT_NAME}）
};

// 模板管理器
class TemplateManager {
public:
    static TemplateManager& instance();
    QList<ConfigTemplate> getTemplates();
    QString applyTemplate(const QString& templateName, const QVariantMap& variables);
};
```

---

### 阶段 2: 用户体验优化 (优先级: 🟡 中)

#### 2.1 配置文件对比功能 ⏳ **待完成**
- [ ] 对比两个配置文件的差异
  - 使用 diff 算法
  - 高亮显示差异部分
- [ ] 配置历史版本管理
  - 自动备份修改前的配置
  - 支持恢复到历史版本
- [ ] 三向合并功能
  - 合并两个配置文件的修改

**技术要点**:
```cpp
// Diff 算法
class ConfigDiff {
public:
    struct DiffLine {
        enum Type { Unchanged, Added, Removed, Modified };
        Type type;
        QString content;
        int lineNumber;
    };

    static QList<DiffLine> compare(const QString& file1, const QString& file2);
};

// 历史版本管理
class ConfigHistory {
public:
    void saveVersion(const QString& filePath, const QString& content);
    QStringList getVersions(const QString& filePath);
    QString getVersion(const QString& filePath, const QDateTime& timestamp);
};
```

---

#### 2.2 快捷键系统 ⏳ **待完成**
- [ ] 全局快捷键
  - `Ctrl+S` - 保存当前配置
  - `Ctrl+W` - 关闭当前标签页
  - `Ctrl+Shift+P` - 快速打开项目
  - `Ctrl+Shift+F` - 全局搜索配置项
  - `Ctrl+,` - 打开设置
- [ ] 编辑器快捷键
  - `Ctrl+F` - 查找
  - `Ctrl+H` - 替换
  - `Ctrl+/` - 注释/取消注释
  - `Ctrl+D` - 复制当前行
  - `Ctrl+Shift+K` - 删除当前行

**技术要点**:
```cpp
// 在 MainWindow 中注册快捷键
void MainWindow::setupShortcuts() {
    // 保存
    QShortcut* saveShortcut = new QShortcut(QKeySequence::Save, this);
    connect(saveShortcut, &QShortcut::activated, this, &MainWindow::saveCurrentConfig);

    // 关闭标签页
    QShortcut* closeShortcut = new QShortcut(QKeySequence::Close, this);
    connect(closeShortcut, &QShortcut::activated, this, &MainWindow::closeCurrentTab);
}
```

---

#### 2.3 搜索和替换功能 ⏳ **待完成**
- [ ] 全局搜索配置项
  - 在所有配置文件中搜索关键字
  - 显示搜索结果列表
  - 点击跳转到对应位置
- [ ] 批量替换
  - 在多个配置文件中批量替换
  - 预览替换结果
  - 支持正则表达式

---

### 阶段 3: 高级功能 (优先级: 🟢 低)

#### 3.1 局域网配置同步 ⏳ **待完成**
- [ ] 设计 `IDataSource` 接口（预留）
  - 本地文件源: `FileDataSource`
  - 网络源: `NetworkDataSource`
- [ ] TCP/UDP 配置广播
  - 主机模式：广播配置变更
  - 从机模式：自动同步配置
- [ ] 配置冲突解决机制
  - 检测冲突
  - 提供合并选项

---

#### 3.2 插件系统 ⏳ **待完成**
- [ ] 自定义配置解析器插件
  - 允许用户扩展支持的配置格式
  - 插件 API 设计
- [ ] 自定义主题插件
  - 允许用户自定义配色方案
  - 主题导入/导出

---

#### 3.3 数据可视化 ⏳ **待完成**
- [ ] 配置项关系图谱
  - 可视化配置依赖关系
  - 使用图形化界面展示
- [ ] 进程资源监控图表
  - CPU 使用率
  - 内存使用率
  - 运行时长
  - 实时更新图表

---

## 🔧 技术债务与优化

### 代码质量
- [ ] 添加单元测试
  - ConfigParser 测试
  - ProcessRunner 测试
  - ParserFactory 测试
- [ ] 添加注释文档
  - 为关键方法添加 Doxygen 注释
  - 生成 API 文档
- [ ] 性能优化
  - 大型配置文件加载优化（分页加载）
  - 日志窗口性能优化（虚拟滚动）
  - 减少不必要的信号发射

### 错误处理
- [ ] 完善异常处理
  - 文件读写失败处理
  - 进程启动失败处理
  - 网络错误处理
- [ ] 用户友好的错误提示
  - 所有 QMessageBox 替换为 ElaMessageBar
  - 提供错误恢复建议

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
- `style: 代码格式调整`
- `test: 测试相关`

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

### v1.0.0-alpha (当前 - 2025-12-15)
- ✅ 基础项目管理
- ✅ 配置文件表单模式（INI/YAML/JSON）
- ✅ 配置文件增量更新策略
- ✅ 配置文件右键菜单
- ✅ 日志系统（主题自适应）
- ✅ 进程监控功能（ProcessRunner 集成）
- ✅ 进程配置持久化
- ✅ 实时日志输出

### v1.1.0-beta (目标: 2周后)
- [ ] 进程状态指示灯
- [ ] 进程日志过滤和导出
- [ ] 配置文件语法高亮
- [ ] 配置文件模板系统

### v1.5.0-rc (目标: 1个月后)
- [ ] 配置对比功能
- [ ] 快捷键系统
- [ ] 搜索和替换功能
- [ ] 配置历史版本管理

### v2.0.0-stable (目标: 2个月后)
- [ ] 局域网配置同步
- [ ] 插件系统
- [ ] 数据可视化
- [ ] 完整的单元测试覆盖

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

**最后更新**: 2025-12-15
**文档版本**: v1.1
