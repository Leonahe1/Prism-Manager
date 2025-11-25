#pragma once

#include <QMainWindow>
#include <QMap>
#include <QString>
#include <memory>

// 前向声明 ElaWidgetTools 类
class ElaWindow;
class ElaTreeView;
class ElaTabWidget;
class ElaTextEdit;
class QSplitter;
class QStandardItemModel;

namespace Prism {

class ProjectManager;
class ProcessRunner;

/**
 * @brief 主窗口
 * 基于 ElaWidgetTools 实现 Fluent Design 风格
 *
 * 布局：
 * - 左侧：项目资源树（带运行状态指示灯）
 * - 中间：多标签页编辑器（支持语法高亮）
 * - 底部：可折叠日志输出面板
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onProjectSelected(const QString& projectName);
    void onRunProject();
    void onStopProject();
    void onOpenConfig();
    void onNewProject();
    void onAbout();

    void onProcessOutput(const QString& output);
    void onProcessError(const QString& error);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupConnections();

    void appendLog(const QString& text, bool isError = false);
    void updateProjectTree();

    // UI 组件
    ElaWindow* m_mainWindow;
    QSplitter* m_mainSplitter;
    QSplitter* m_verticalSplitter;

    // 左侧项目树
    ElaTreeView* m_projectTreeView;
    QStandardItemModel* m_projectTreeModel;

    // 中间编辑区
    ElaTabWidget* m_tabWidget;

    // 底部日志面板
    ElaTextEdit* m_logOutput;
    bool m_logVisible;

    // 业务逻辑
    std::unique_ptr<ProjectManager> m_projectManager;
    QString m_currentProject;
};

} // namespace Prism
