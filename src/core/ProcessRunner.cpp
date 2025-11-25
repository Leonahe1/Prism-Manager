#include "ProcessRunner.h"
#include <QTextCodec>
#include <QDebug>

namespace Prism {

ProcessRunner::ProcessRunner(QObject* parent)
    : QObject(parent)
    , m_process(std::make_unique<QProcess>(this))
    , m_state(ProcessState::NotStarted)
    , m_exitCode(0)
{
    // 连接 QProcess 信号
    connect(m_process.get(), &QProcess::readyReadStandardOutput,
            this, &ProcessRunner::onReadyReadStandardOutput);
    connect(m_process.get(), &QProcess::readyReadStandardError,
            this, &ProcessRunner::onReadyReadStandardError);
    connect(m_process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProcessRunner::onFinished);
    connect(m_process.get(), &QProcess::errorOccurred,
            this, &ProcessRunner::onErrorOccurred);
    connect(m_process.get(), &QProcess::started,
            this, &ProcessRunner::onStarted);
}

ProcessRunner::~ProcessRunner() {
    if (isRunning()) {
        stop(true);
    }
}

bool ProcessRunner::start(const QString& program, const QStringList& arguments, const QString& workingDirectory) {
    if (isRunning()) {
        qWarning() << "[ProcessRunner] Process already running";
        return false;
    }

    m_program = program;
    m_arguments = arguments;
    m_exitCode = 0;

    if (!workingDirectory.isEmpty()) {
        m_process->setWorkingDirectory(workingDirectory);
    }

    // 合并标准输出和错误输出的通道（可选）
    // m_process->setProcessChannelMode(QProcess::MergedChannels);

    setState(ProcessState::NotStarted);

    qDebug() << "[ProcessRunner] Starting:" << program << arguments;
    m_process->start(program, arguments);

    return true;
}

void ProcessRunner::stop(bool forceKill) {
    if (!isRunning()) {
        return;
    }

    qDebug() << "[ProcessRunner] Stopping process (forceKill=" << forceKill << ")";

    if (forceKill) {
        m_process->kill();
    } else {
        m_process->terminate();
        // 等待 3 秒后强制杀死
        if (!m_process->waitForFinished(3000)) {
            qWarning() << "[ProcessRunner] Process did not terminate gracefully, killing...";
            m_process->kill();
            m_process->waitForFinished(1000);
        }
    }

    setState(ProcessState::Killed);
}

bool ProcessRunner::isRunning() const {
    return m_process && m_process->state() == QProcess::Running;
}

qint64 ProcessRunner::processId() const {
    return m_process ? m_process->processId() : 0;
}

bool ProcessRunner::waitForFinished(int msecs) {
    return m_process && m_process->waitForFinished(msecs);
}

void ProcessRunner::onReadyReadStandardOutput() {
    QByteArray data = m_process->readAllStandardOutput();
    if (!data.isEmpty()) {
        QString output = convertEncoding(data);
        emit standardOutput(output);
    }
}

void ProcessRunner::onReadyReadStandardError() {
    QByteArray data = m_process->readAllStandardError();
    if (!data.isEmpty()) {
        QString error = convertEncoding(data);
        emit standardError(error);
    }
}

void ProcessRunner::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_exitCode = exitCode;

    qDebug() << "[ProcessRunner] Process finished with code" << exitCode
             << "status" << (exitStatus == QProcess::NormalExit ? "Normal" : "Crashed");

    setState(exitStatus == QProcess::NormalExit ? ProcessState::Finished : ProcessState::Error);
    emit finished(exitCode, exitStatus);
}

void ProcessRunner::onErrorOccurred(QProcess::ProcessError error) {
    QString errorString;

    switch (error) {
        case QProcess::FailedToStart:
            errorString = QString("Failed to start: %1").arg(m_program);
            break;
        case QProcess::Crashed:
            errorString = "Process crashed";
            break;
        case QProcess::Timedout:
            errorString = "Process timed out";
            break;
        case QProcess::WriteError:
            errorString = "Write error";
            break;
        case QProcess::ReadError:
            errorString = "Read error";
            break;
        case QProcess::UnknownError:
        default:
            errorString = "Unknown error";
            break;
    }

    qWarning() << "[ProcessRunner]" << errorString;
    setState(ProcessState::Error);
    emit errorOccurred(errorString);
}

void ProcessRunner::onStarted() {
    qDebug() << "[ProcessRunner] Process started, PID:" << processId();
    setState(ProcessState::Running);
    emit started();
}

QString ProcessRunner::convertEncoding(const QByteArray& data) {
#ifdef Q_OS_WIN
    // Windows 下，控制台输出通常是 GBK 编码
    // 尝试使用 GBK 编解码器转换为 UTF-8
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    if (codec) {
        return codec->toUnicode(data);
    }
#endif

    // Linux/macOS 通常已经是 UTF-8
    return QString::fromUtf8(data);
}

void ProcessRunner::setState(ProcessState newState) {
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(m_state);
    }
}

} // namespace Prism
