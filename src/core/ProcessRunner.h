#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <memory>

namespace Prism {

/**
 * @brief 进程运行器
 * 封装 QProcess，提供实时日志捕获、编码转换、进程控制等功能
 */
class ProcessRunner : public QObject {
    Q_OBJECT

public:
    enum class ProcessState {
        NotStarted,
        Starting,   // 启动中（新增）
        Running,
        Finished,
        Error,
        Killed
    };

    explicit ProcessRunner(QObject* parent = nullptr);
    ~ProcessRunner() override;

    /**
     * @brief 启动进程
     * @param program 程序路径或命令
     * @param arguments 命令行参数
     * @param workingDirectory 工作目录（可选）
     * @param showConsole 是否显示控制台窗口（仅 Windows 有效）
     * @return 是否成功启动
     */
    bool start(const QString& program, const QStringList& arguments = QStringList(),
               const QString& workingDirectory = QString(), bool showConsole = false);

    /**
     * @brief 终止进程
     * @param forceKill 是否强制杀死（默认先尝试优雅终止）
     */
    void stop(bool forceKill = false);

    /**
     * @brief 获取当前状态
     */
    ProcessState getState() const { return m_state; }

    /**
     * @brief 进程是否正在运行
     */
    bool isRunning() const;

    /**
     * @brief 获取进程 ID
     */
    qint64 processId() const;

    /**
     * @brief 获取退出码
     */
    int exitCode() const { return m_exitCode; }

    /**
     * @brief 等待进程结束
     * @param msecs 超时时间（毫秒）
     * @return 是否成功结束
     */
    bool waitForFinished(int msecs = 30000);

signals:
    /**
     * @brief 标准输出信号（已转换为 UTF-8）
     * @param output 输出内容
     */
    void standardOutput(const QString& output);

    /**
     * @brief 标准错误输出信号（已转换为 UTF-8）
     * @param error 错误内容
     */
    void standardError(const QString& error);

    /**
     * @brief 进程状态变化信号
     * @param state 新状态
     */
    void stateChanged(ProcessState state);

    /**
     * @brief 进程启动信号
     */
    void started();

    /**
     * @brief 进程结束信号
     * @param exitCode 退出码
     * @param exitStatus 退出状态
     */
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief 进程错误信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);
    void onStarted();

private:

    void setState(ProcessState newState);

    std::unique_ptr<QProcess> m_process;
    ProcessState m_state;
    int m_exitCode;
    QString m_program;
    QStringList m_arguments;
};

} // namespace Prism
