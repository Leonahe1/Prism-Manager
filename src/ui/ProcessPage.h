#ifndef PROCESSPAGE_H
#define PROCESSPAGE_H

#include "BasePage.h"
#include "ProcessEditDialog.h"
#include "ProcessStatusIndicator.h"
#include "ElaDef.h"
#include <QString>
#include <QProcess>
#include <QMap>
#include <QList>
#include <memory>

// 前向声明
class ElaPlainTextEdit;
class ElaPushButton;
class ElaTabWidget;
class ElaTableView;
class QVBoxLayout;
class QStandardItemModel;

namespace Prism {

class ProcessRunner;
class LogToolBar;

/**
 * @brief 进程表格行数据
 */
struct ProcessRowData {
    ProcessConfig config;
    std::shared_ptr<ProcessRunner> runner;
    ProcessStatusIndicator::Status currentStatus{ ProcessStatusIndicator::Status::Stopped };
};

/**
 * @brief 日志条目结构
 */
struct LogEntry {
    QString timestamp;
    QString level;
    QString configId;
    QString message;
};

/**
 * @brief 多进程监控页面（表格式布局）
 *
 * 布局设计（参考 Supervisor）：
 * - 顶部：工具栏（添加进程、全部启动、全部停止）
 * - 中间：进程表格（名称、程序路径、状态、操作按钮）
 * - 底部：日志 Tab 页（全部 + 各进程独立日志）
 */
class ProcessPage : public BasePage
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit ProcessPage(QWidget* parent = nullptr);
    ~ProcessPage() override;

    /**
     * @brief 设置当前项目名称
     */
    void setProjectName(const QString& projectName);

    /**
     * @brief 获取当前运行中的进程数量
     */
    int getRunningProcessCount() const;

public slots:
    /**
     * @brief 添加新进程
     */
    void addProcess();

    /**
     * @brief 编辑进程
     * @param row 表格行索引
     */
    void editProcess(int row);

    /**
     * @brief 删除进程
     * @param row 表格行索引
     */
    void deleteProcess(int row);

    /**
     * @brief 启动进程
     * @param row 表格行索引
     */
    void startProcess(int row);

    /**
     * @brief 停止进程
     * @param row 表格行索引
     */
    void stopProcess(int row);

    /**
     * @brief 启动所有进程
     */
    void startAllProcesses();

    /**
     * @brief 停止所有进程
     */
    void stopAllProcesses();

    /**
     * @brief 清空当前日志
     */
    void clearCurrentLog();

    /**
     * @brief 清空所有日志
     */
    void clearAllLogs();

    /**
     * @brief 设置日志过滤级别
     */
    void setLogFilter(const QStringList& levels);

    /**
     * @brief 搜索日志
     */
    void searchLogs(const QString& keyword);

    /**
     * @brief 导出日志
     */
    void exportLogs();

signals:
    void processStarted(const QString& projectName, const QString& processName);
    void processStopped(const QString& projectName, const QString& processName, int exitCode);

private slots:
    void onThemeChanged(ElaThemeType::ThemeMode mode);

private:
    // 初始化
    void initUI();
    void setupConnections();

    // 表格操作
    void addTableRow(const ProcessConfig& config);
    void updateTableRow(int row);
    void removeTableRow(int row);
    QWidget* createOperationWidget(int row);
    void updateOperationButtons(int row, ProcessStatusIndicator::Status status);
    void updateStatusIndicator(int row, ProcessStatusIndicator::Status status);

    // 进程管理
    void connectProcessSignals(int row);
    void onProcessStarted(const QString& configId);
    void onProcessFinished(const QString& configId, int exitCode, QProcess::ExitStatus status);
    void onProcessError(const QString& configId, const QString& error);
    void onProcessOutput(const QString& configId, const QString& output, bool isError);

    // 日志管理
    ElaPlainTextEdit* createLogTextEdit();
    void addLogTab(const QString& configId, const QString& name);
    void removeLogTab(const QString& configId);
    void updateLogTabName(const QString& configId, const QString& newName);
    void appendLog(ElaPlainTextEdit* textEdit, const QString& level, const QString& message);
    void appendLogToTab(const QString& configId, const QString& level, const QString& message);
    void updateLogStyle(ElaPlainTextEdit* textEdit);

    // 日志增强
    void trimLogLines(ElaPlainTextEdit* textEdit);
    void refreshLogDisplay();
    void highlightSearchResults(ElaPlainTextEdit* textEdit, const QString& keyword);
    void clearSearchHighlight(ElaPlainTextEdit* textEdit);
    void storeLogEntry(const QString& configId, const QString& level, const QString& message);
    bool shouldShowLogEntry(const LogEntry& entry) const;

    // 持久化
    void saveProcessConfig();
    void loadProcessConfig();

    // 辅助方法
    int findRowByConfigId(const QString& configId);
    void updateGlobalButtonStates();

private:
    // ========================================
    // UI 组件 - 工具栏
    // ========================================
    ElaPushButton* _addProcessButton{ nullptr };
    ElaPushButton* _startAllButton{ nullptr };
    ElaPushButton* _stopAllButton{ nullptr };

    // ========================================
    // UI 组件 - 进程表格
    // ========================================
    ElaTableView* _processTable{ nullptr };
    QStandardItemModel* _processTableModel{ nullptr };

    // ========================================
    // UI 组件 - 日志区域
    // ========================================
    LogToolBar* _logToolBar{ nullptr };
    ElaTabWidget* _logTabWidget{ nullptr };
    ElaPlainTextEdit* _allLogTextEdit{ nullptr };
    QMap<QString, ElaPlainTextEdit*> _logTextEdits;  // configId -> 日志编辑框
    QMap<QString, int> _logTabIndices;               // configId -> Tab 索引

    // ========================================
    // UI 组件 - 底部按钮
    // ========================================
    ElaPushButton* _clearCurrentLogButton{ nullptr };
    ElaPushButton* _clearAllLogButton{ nullptr };

    // ========================================
    // 数据管理
    // ========================================
    QString _currentProjectName;
    QList<ProcessRowData> _processData;  // 进程数据列表

    // 日志颜色方案
    QMap<QString, QString> m_darkColors;
    QMap<QString, QString> m_lightColors;

    // 日志增强数据
    QList<LogEntry> _logBuffer;           // 日志缓存
    QStringList _activeFilterLevels;      // 当前过滤级别
    QString _searchKeyword;               // 搜索关键字
    static const int MAX_LOG_LINES = 10000;  // 最大日志行数
};

} // namespace Prism

#endif // PROCESSPAGE_H
