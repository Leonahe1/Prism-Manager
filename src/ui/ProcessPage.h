#ifndef PROCESSPAGE_H
#define PROCESSPAGE_H

#include "BasePage.h"
#include <QString>
#include <QProcess>

// 前向声明
class ElaPlainTextEdit;
class ElaPushButton;
class ElaLineEdit;
class ElaProgressBar;
class QVBoxLayout;

namespace Prism {

class ProcessRunner;

/**
 * @brief 进程监控页面
 *
 * 职责：
 * - 顶部：运行命令输入框 + 参数输入框
 * - 中间：日志输出区域（黑色终端风格）
 * - 底部：运行/停止按钮 + 进程状态指示
 *
 * 设计要点：
 * - 实时捕获 stdout 和 stderr
 * - 支持一键启动和强制终止
 * - 自动编码转换（GBK → UTF-8 on Windows）
 * - 进程状态显示（运行中/已停止/错误）
 */
class ProcessPage : public BasePage
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit ProcessPage(QWidget* parent = nullptr);
    ~ProcessPage() override;

    /**
     * @brief 设置当前项目名称
     * @param projectName 项目名称（用于从 ProjectManager 获取 ProcessRunner）
     */
    void setProjectName(const QString& projectName);

    /**
     * @brief 设置要运行的程序路径
     * @param program 程序路径
     */
    void setProgram(const QString& program);

    /**
     * @brief 设置命令行参数
     * @param arguments 参数列表
     */
    void setArguments(const QStringList& arguments);

public slots:
    /**
     * @brief 启动进程
     */
    void startProcess();

    /**
     * @brief 停止进程
     * @param forceKill 是否强制杀死（默认 false，先尝试优雅退出）
     */
    void stopProcess(bool forceKill = false);

    /**
     * @brief 清空日志
     */
    void clearLog();

    /**
     * @brief 追加日志输出
     * @param message 日志消息
     * @param isError 是否为错误信息（红色显示）
     */
    void appendLog(const QString& message, bool isError = false);

signals:
    /**
     * @brief 进程启动信号
     * @param projectName 项目名称
     */
    void processStarted(const QString& projectName);

    /**
     * @brief 进程停止信号
     * @param projectName 项目名称
     * @param exitCode 退出码
     */
    void processStopped(const QString& projectName, int exitCode);

    /**
     * @brief 进程错误信号
     * @param projectName 项目名称
     * @param error 错误信息
     */
    void processError(const QString& projectName, const QString& error);

private:
    // ========================================
    // 初始化方法
    // ========================================
    void initUI();
    void setupConnections();

    // ========================================
    // 辅助方法
    // ========================================
    /**
     * @brief 更新进程状态 UI
     * @param isRunning 是否正在运行
     */
    void updateProcessStatus(bool isRunning);

    /**
     * @brief 格式化日志时间戳
     * @return 格式化的时间字符串 [HH:mm:ss]
     */
    QString getTimestamp();

private slots:
    // ========================================
    // 进程信号槽
    // ========================================
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onProcessReadyReadStandardOutput();
    void onProcessReadyReadStandardError();

private:
    // ========================================
    // UI 组件
    // ========================================
    ElaLineEdit* _programLineEdit{ nullptr };      // 程序路径输入框
    ElaLineEdit* _argumentsLineEdit{ nullptr };    // 参数输入框
    ElaPushButton* _startButton{ nullptr };        // 启动按钮
    ElaPushButton* _stopButton{ nullptr };         // 停止按钮
    ElaPushButton* _clearButton{ nullptr };        // 清空日志按钮
    ElaPlainTextEdit* _logTextEdit{ nullptr };     // 日志输出区域
    ElaProgressBar* _statusBar{ nullptr };         // 状态指示条（可选）

    // ========================================
    // 数据管理
    // ========================================
    QString _currentProjectName;                   // 当前项目名称
    QProcess* _process{ nullptr };                 // 进程对象
    bool _isRunning{ false };                      // 是否正在运行

    // TODO: 后续集成 ProcessRunner
    // std::shared_ptr<ProcessRunner> _processRunner;
};

} // namespace Prism

#endif // PROCESSPAGE_H
