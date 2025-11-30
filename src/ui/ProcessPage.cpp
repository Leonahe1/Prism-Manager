#include "ProcessPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QTextCodec>
#include <QDebug>

#include "ElaPlainTextEdit.h"
#include "ElaPushButton.h"
#include "ElaLineEdit.h"
#include "ElaText.h"
#include "ElaMessageBar.h"

namespace Prism {

ProcessPage::ProcessPage(QWidget* parent)
    : BasePage(parent)
{
    setWindowTitle("进程监控");
    initUI();
    setupConnections();

    _process = new QProcess(this);

    qDebug() << "ProcessPage 初始化完成";
}

ProcessPage::~ProcessPage()
{
    if (_process && _process->state() == QProcess::Running) {
        _process->kill();
        _process->waitForFinished(3000);
    }
}

void ProcessPage::initUI()
{
    // ========================================
    // 顶部：命令输入区域
    // ========================================
    QWidget* topWidget = new QWidget(this);
    QVBoxLayout* topLayout = new QVBoxLayout(topWidget);
    topLayout->setContentsMargins(10, 5, 10, 5);

    // 程序路径输入
    QHBoxLayout* programLayout = new QHBoxLayout();
    ElaText* programLabel = new ElaText("程序路径:", this);
    programLabel->setTextPixelSize(14);
    programLabel->setFixedWidth(80);
    _programLineEdit = new ElaLineEdit(this);
    _programLineEdit->setPlaceholderText("输入可执行文件路径，例如: ./simulator.exe");
    programLayout->addWidget(programLabel);
    programLayout->addWidget(_programLineEdit);

    // 参数输入
    QHBoxLayout* argumentsLayout = new QHBoxLayout();
    ElaText* argumentsLabel = new ElaText("命令参数:", this);
    argumentsLabel->setTextPixelSize(14);
    argumentsLabel->setFixedWidth(80);
    _argumentsLineEdit = new ElaLineEdit(this);
    _argumentsLineEdit->setPlaceholderText("输入命令行参数，例如: --config=config.yaml --verbose");
    argumentsLayout->addWidget(argumentsLabel);
    argumentsLayout->addWidget(_argumentsLineEdit);

    topLayout->addLayout(programLayout);
    topLayout->addLayout(argumentsLayout);

    // ========================================
    // 中间：日志输出区域（黑色终端风格）
    // ========================================
    QWidget* middleWidget = new QWidget(this);
    QVBoxLayout* middleLayout = new QVBoxLayout(middleWidget);
    middleLayout->setContentsMargins(10, 5, 10, 5);

    ElaText* logTitle = new ElaText("日志输出", this);
    logTitle->setTextPixelSize(16);
    middleLayout->addWidget(logTitle);

    _logTextEdit = new ElaPlainTextEdit(this);
    _logTextEdit->setReadOnly(true);
    _logTextEdit->setStyleSheet(
        "ElaPlainTextEdit { "
        "background-color: #1E1E1E; "
        "color: #D4D4D4; "
        "font-family: 'Consolas', 'Courier New', monospace; "
        "font-size: 11pt; "
        "}"
    );
    _logTextEdit->appendPlainText("[INFO] 进程监控页面已就绪");
    _logTextEdit->appendPlainText("[INFO] 请输入程序路径后点击「启动」按钮");
    middleLayout->addWidget(_logTextEdit);

    // ========================================
    // 底部：按钮区域
    // ========================================
    QWidget* bottomWidget = new QWidget(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout(bottomWidget);
    buttonLayout->setContentsMargins(10, 5, 10, 10);
    buttonLayout->addStretch();

    _clearButton = new ElaPushButton("清空日志", this);
    _clearButton->setFixedWidth(100);
    buttonLayout->addWidget(_clearButton);

    _stopButton = new ElaPushButton("停止", this);
    _stopButton->setFixedWidth(100);
    _stopButton->setEnabled(false);
    buttonLayout->addWidget(_stopButton);

    _startButton = new ElaPushButton("启动", this);
    _startButton->setFixedWidth(100);
    buttonLayout->addWidget(_startButton);

    // ========================================
    // 组装到页面中央
    // ========================================
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("进程监控");  // 设置标题，避免显示 Page_0
    QVBoxLayout* centerLayout = new QVBoxLayout(centralWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->addWidget(topWidget);
    centerLayout->addWidget(middleWidget, 1); // 日志区域占据主要空间
    centerLayout->addWidget(bottomWidget);

    // 禁用垂直拖拽手势和鼠标延迟，避免干扰输入操作
    addCentralWidget(centralWidget, true, false, 0);
}

void ProcessPage::setupConnections()
{
    // 按钮点击
    connect(_startButton, &ElaPushButton::clicked, this, &ProcessPage::startProcess);
    connect(_stopButton, &ElaPushButton::clicked, this, [this]() {
        stopProcess(false);
    });
    connect(_clearButton, &ElaPushButton::clicked, this, &ProcessPage::clearLog);

    // 进程信号
    connect(_process, &QProcess::started, this, &ProcessPage::onProcessStarted);
    connect(_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProcessPage::onProcessFinished);
    connect(_process, &QProcess::errorOccurred, this, &ProcessPage::onProcessError);
    connect(_process, &QProcess::readyReadStandardOutput, this, &ProcessPage::onProcessReadyReadStandardOutput);
    connect(_process, &QProcess::readyReadStandardError, this, &ProcessPage::onProcessReadyReadStandardError);
}

void ProcessPage::setProjectName(const QString& projectName)
{
    _currentProjectName = projectName;
    qDebug() << "ProcessPage 设置项目名称:" << projectName;
}

void ProcessPage::setProgram(const QString& program)
{
    _programLineEdit->setText(program);
}

void ProcessPage::setArguments(const QStringList& arguments)
{
    _argumentsLineEdit->setText(arguments.join(" "));
}

void ProcessPage::startProcess()
{
    QString program = _programLineEdit->text().trimmed();
    if (program.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "请输入程序路径", 1500);
        return;
    }

    if (_isRunning) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "进程已在运行中", 1500);
        return;
    }

    // 解析参数
    QString argumentsStr = _argumentsLineEdit->text().trimmed();
    QStringList arguments;
    if (!argumentsStr.isEmpty()) {
        arguments = argumentsStr.split(' ', QString::SkipEmptyParts);
    }

    appendLog(QString("[启动] 程序: %1").arg(program), false);
    if (!arguments.isEmpty()) {
        appendLog(QString("[启动] 参数: %1").arg(arguments.join(" ")), false);
    }

    _process->start(program, arguments);

    // 等待启动（最多 3 秒）
    if (!_process->waitForStarted(3000)) {
        appendLog("[错误] 进程启动失败", true);
        ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                             "进程启动失败", 2000);
    }
}

void ProcessPage::stopProcess(bool forceKill)
{
    if (!_isRunning) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "进程未运行", 1500);
        return;
    }

    appendLog("[停止] 正在终止进程...", false);

    if (forceKill) {
        _process->kill();
    } else {
        _process->terminate();
        // 等待 3 秒，如果还没结束就强制杀死
        if (!_process->waitForFinished(3000)) {
            appendLog("[警告] 进程未响应，强制杀死", true);
            _process->kill();
        }
    }
}

void ProcessPage::clearLog()
{
    _logTextEdit->clear();
    appendLog("[INFO] 日志已清空", false);
}

void ProcessPage::appendLog(const QString& message, bool isError)
{
    QString timestamp = getTimestamp();
    QString formattedMessage = QString("%1 %2").arg(timestamp, message);

    if (isError) {
        // TODO: 使用 HTML 或 setTextColor 显示红色
        _logTextEdit->appendPlainText(formattedMessage);
    } else {
        _logTextEdit->appendPlainText(formattedMessage);
    }

    // 自动滚动到底部
    _logTextEdit->moveCursor(QTextCursor::End);
}

void ProcessPage::updateProcessStatus(bool isRunning)
{
    _isRunning = isRunning;
    _startButton->setEnabled(!isRunning);
    _stopButton->setEnabled(isRunning);

    if (isRunning) {
        _programLineEdit->setEnabled(false);
        _argumentsLineEdit->setEnabled(false);
    } else {
        _programLineEdit->setEnabled(true);
        _argumentsLineEdit->setEnabled(true);
    }
}

QString ProcessPage::getTimestamp()
{
    return QDateTime::currentDateTime().toString("[HH:mm:ss]");
}

// ========================================
// 进程信号槽
// ========================================
void ProcessPage::onProcessStarted()
{
    updateProcessStatus(true);
    appendLog("[成功] 进程已启动", false);
    ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                           "进程已启动", 1500);

    emit processStarted(_currentProjectName);
}

void ProcessPage::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    updateProcessStatus(false);

    if (exitStatus == QProcess::NormalExit) {
        appendLog(QString("[完成] 进程正常退出，退出码: %1").arg(exitCode), false);
        ElaMessageBar::information(ElaMessageBarType::BottomRight, "信息",
                                   QString("进程已退出 (代码: %1)").arg(exitCode), 2000);
    } else {
        appendLog("[错误] 进程异常终止", true);
        ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                             "进程异常终止", 2000);
    }

    emit processStopped(_currentProjectName, exitCode);
}

void ProcessPage::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "启动失败：程序未找到或权限不足";
        break;
    case QProcess::Crashed:
        errorMsg = "进程崩溃";
        break;
    case QProcess::Timedout:
        errorMsg = "操作超时";
        break;
    case QProcess::WriteError:
        errorMsg = "写入错误";
        break;
    case QProcess::ReadError:
        errorMsg = "读取错误";
        break;
    case QProcess::UnknownError:
    default:
        errorMsg = "未知错误";
        break;
    }

    appendLog(QString("[错误] %1").arg(errorMsg), true);
    ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误", errorMsg, 2000);

    emit processError(_currentProjectName, errorMsg);
}

void ProcessPage::onProcessReadyReadStandardOutput()
{
    QByteArray data = _process->readAllStandardOutput();

    // Windows 编码转换 (GBK → UTF-8)
#ifdef Q_OS_WIN
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    QString output = codec->toUnicode(data);
#else
    QString output = QString::fromUtf8(data);
#endif

    // 按行输出
    QStringList lines = output.split('\n', QString::SkipEmptyParts);
    for (const QString& line : lines) {
        appendLog(QString("[stdout] %1").arg(line.trimmed()), false);
    }
}

void ProcessPage::onProcessReadyReadStandardError()
{
    QByteArray data = _process->readAllStandardError();

    // Windows 编码转换 (GBK → UTF-8)
#ifdef Q_OS_WIN
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    QString output = codec->toUnicode(data);
#else
    QString output = QString::fromUtf8(data);
#endif

    // 按行输出（错误信息）
    QStringList lines = output.split('\n', QString::SkipEmptyParts);
    for (const QString& line : lines) {
        appendLog(QString("[stderr] %1").arg(line.trimmed()), true);
    }
}

} // namespace Prism
