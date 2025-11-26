#pragma once
#include "ElaWindow.h"

#include <QMainWindow>
#include <QString>
#include <QMap>
#include <QDateTime>
#include <memory>

// 前向声明 ElaWidgetTools 类
class ElaStatusBar;
class ElaContentDialog;
class ElaMenuBar;

// 前向声明页面类
class HomePage;
class BasePage;

namespace Prism {

class ProjectManager;
class ProcessRunner;

/**
 * @brief 项目信息结构
 *
 * 用于管理单个项目的基本信息和状态
 */
struct ProjectInfo {
    QString id;              // 唯一标识符 (UUID)
    QString name;            // 项目名称
    QString rootPath;        // 项目根目录
    QStringList configFiles; // 配置文件列表
    QDateTime lastOpened;    // 最后打开时间
    bool isActive;           // 是否为当前激活项目
};

/**
 * @brief 主窗口
 * 基于 ElaWidgetTools 实现 Fluent Design 风格
 *
 * 架构设计：
 * - 使用 ElaWindow 的导航系统管理页面
 * - 左侧：自动生成的导航栏（带图标和展开/折叠）
 * - 中间：页面内容区域（通过 addPageNode 注册）
 * - 底部：可选的 DockWidget 日志面板
 */
class MainWindow : public ElaWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // 初始化方法（分离关注点）
    void initWindow();      // 窗口基础配置
    void initEdgeLayout();  // 菜单栏/工具栏/状态栏/停靠窗口
    void initContent();     // 核心页面内容

    // ========================================
    // 项目管理公共接口
    // ========================================
    /**
     * @brief 添加新项目到导航树
     * @param projectPath 项目根目录路径
     * @return 项目 ID (失败返回空字符串)
     */
    QString addProject(const QString& projectPath);

    /**
     * @brief 关闭项目（移除导航节点）
     * @param projectId 项目 ID
     */
    void closeProject(const QString& projectId);

    /**
     * @brief 切换当前激活项目
     * @param projectId 项目 ID
     */
    void setActiveProject(const QString& projectId);

private slots:
    // ========================================
    // 菜单栏槽函数
    // ========================================
    // 文件菜单
    void onNewProject();          // 新建项目 (Ctrl+N)
    void onOpenProject();         // 打开项目 (Ctrl+O)
    void onSaveConfig();          // 保存配置 (Ctrl+S)
    void onCloseCurrentProject(); // 关闭当前项目 (Ctrl+W)
    void onExit();                // 退出 (Ctrl+Q)

    // 项目菜单
    void onProjectSettings();     // 项目设置
    void onAddConfigFile();       // 添加配置文件
    void onRefreshProject();      // 刷新项目
    void onOpenInExplorer();      // 在资源管理器中打开

private:
    // ========================================
    // 页面管理
    // ========================================
    HomePage* _homePage{ nullptr };
    BasePage* _settingPage{ nullptr };

    // ========================================
    // 多项目管理数据结构
    // ========================================
    QMap<QString, ProjectInfo> _openedProjects;        // 已打开的项目
    QMap<QString, QString> _projectExpanderKeys;       // 项目ID -> 展开节点Key
    QMap<QString, BasePage*> _projectConfigPages;      // 项目ID -> 配置页面
    QMap<QString, BasePage*> _projectProcessPages;     // 项目ID -> 进程页面
    QString _currentProjectId;                         // 当前激活的项目ID

    // ========================================
    // 导航 Key
    // ========================================
    QString _projectManagementKey{ "" }; // "项目管理"展开节点的 Key
    QString _settingKey{ "" };

    // ========================================
    // UI 组件
    // ========================================
    ElaMenuBar* _menuBar{ nullptr };
    ElaStatusBar* _statusBar{ nullptr };
    ElaContentDialog* _closeDialog{ nullptr };

    // ========================================
    // 业务逻辑
    // ========================================
    std::unique_ptr<ProjectManager> m_projectManager;

    // ========================================
    // 辅助方法（预留）
    // ========================================
    // TODO: 添加项目持久化方法
    // void saveProjectsToConfig();
    // void loadProjectsFromConfig();

    // TODO: 添加最近项目管理
    // QStringList getRecentProjects();
    // void updateRecentProjects(const QString& projectId);
};

} // namespace Prism
