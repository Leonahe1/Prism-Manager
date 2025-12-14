#include "ProcessPage.h"
#include "core/ProcessRunner.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QTextCodec>
#include <QTextCursor>
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QTimer>

#include "ElaPlainTextEdit.h"
#include "ElaPushButton.h"
#include "ElaLineEdit.h"
#include "ElaText.h"
#include "ElaMessageBar.h"
#include "ElaTheme.h"

namespace Prism {

ProcessPage::ProcessPage(QWidget* parent)
    : BasePage(parent)
{
    setWindowTitle("进程监控");

    // 定义日志级别对应的颜色（深色主题）
    m_darkColors["INFO"] = "#61AFEF";      // 蓝色
    m_darkColors["SUCCESS"] = "#98C379";   // 绿色
    m_darkColors["WARNING"] = "#E5C07B";   // 黄色
    m_darkColors["ERROR"] = "#E06C75";     // 红色
    m_darkColors["DEBUG"] = "#C678DD";     // 紫色
    m_darkColors["PROCESS"] = "#56B6C2";   // 青色
    m_darkColors["STDOUT"] = "#ABB2BF";    // 灰白
    m_darkColors["STDERR"] = "#E06C75";    // 红色

    // 定义日志级别对应的颜色（浅色主题）
    m_lightColors["INFO"] = "#0078D4";     // 蓝色
    m_lightColors["SUCCESS"] = "#107C10";  // 绿色
    m_lightColors["WARNING"] = "#F7630C";  // 橙色
    m_lightColors["ERROR"] = "#D13438";    // 红色
    m_lightColors["DEBUG"] = "#8764B8";    // 紫色
    m_lightColors["PROCESS"] = "#00979C";  // 青色
    m_lightColors["STDOUT"] = "#323130";   // 深灰
    m_lightColors["STDERR"] = "#D13438";   // 红色

    initUI();

    // 创建 ProcessRunner 实例
    _processRunner = std::make_shared<ProcessRunner>(this);

    setupConnections();

    // 连接主题切换信号
    connect(eTheme, &ElaTheme::themeModeChanged, this, &ProcessPage::onThemeChanged);

    // 加载持久化配置
    loadProcessConfig();

    qDebug() << "ProcessPage 初始化完成";
}

ProcessPage::~ProcessPage()
{
    if (_processRunner && _processRunner->isRunning()) {
        _processRunner->stop(true);  // 强制停止
        _processRunner->waitForFinished(3000);
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
    _browseButton = new ElaPushButton("浏览...", this);
    _browseButton->setFixedWidth(80);
    programLayout->addWidget(programLabel);
    programLayout->addWidget(_programLineEdit);
    programLayout->addWidget(_browseButton);

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

    // 根据当前主题设置样式
    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;
    if (isDark) {
        _logTextEdit->setStyleSheet(
            "ElaPlainTextEdit { "
            "background-color: #1E1E1E; "
            "color: #D4D4D4; "
            "font-family: 'Consolas', 'Courier New', monospace; "
            "font-size: 11pt; "
            "}"
        );
    } else {
        _logTextEdit->setStyleSheet(
            "ElaPlainTextEdit { "
            "background-color: #FFFFFF; "
            "color: #323130; "
            "font-family: 'Consolas', 'Courier New', monospace; "
            "font-size: 11pt; "
            "border: 1px solid #E1E1E1; "
            "}"
        );
    }

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

    // 添加初始日志消息
    appendLog("INFO", "进程监控页面已就绪");
    appendLog("INFO", "请输入程序路径后点击「启动」按钮");
}

void ProcessPage::setupConnections()
{
    // 按钮点击
    connect(_startButton, &ElaPushButton::clicked, this, &ProcessPage::startProcess);
    connect(_stopButton, &ElaPushButton::clicked, this, [this]() {
        stopProcess(false);
    });
    connect(_clearButton, &ElaPushButton::clicked, this, &ProcessPage::clearLog);
    connect(_browseButton, &ElaPushButton::clicked, this, &ProcessPage::browseProgram);

    // 输入框变化时保存配置
    connect(_programLineEdit, &ElaLineEdit::textChanged, this, &ProcessPage::saveProcessConfig);
    connect(_argumentsLineEdit, &ElaLineEdit::textChanged, this, &ProcessPage::saveProcessConfig);

    // ========================================
    // ProcessRunner 信号连接
    // ========================================

    // 标准输出信号（已自动转换编码）
    connect(_processRunner.get(), &ProcessRunner::standardOutput, this, [this](const QString& output) {
        // 按行分割输出
        QStringList lines = output.split('\n', QString::SkipEmptyParts);
        for (const QString& line : lines) {
            QString trimmedLine = line.trimmed();
            if (!trimmedLine.isEmpty()) {
                appendLog("STDOUT", trimmedLine);
            }
        }
    });

    // 标准错误输出信号（已自动转换编码）
    connect(_processRunner.get(), &ProcessRunner::standardError, this, [this](const QString& error) {
        // 按行分割输出
        QStringList lines = error.split('\n', QString::SkipEmptyParts);
        for (const QString& line : lines) {
            QString trimmedLine = line.trimmed();
            if (!trimmedLine.isEmpty()) {
                appendLog("STDERR", trimmedLine);
            }
        }
    });

    // 进程启动信号
    connect(_processRunner.get(), &ProcessRunner::started, this, [this]() {
        updateProcessStatus(true);
        appendLog("SUCCESS", "进程已启动");
        ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                               "进程已启动", 1500);
        emit processStarted(_currentProjectName);
    });

    // 进程结束信号
    connect(_processRunner.get(), &ProcessRunner::finished, this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
        updateProcessStatus(false);

        // 检查是否是用户主动停止
        bool isUserStopped = (_processRunner->getState() == ProcessRunner::ProcessState::Killed);

        if (isUserStopped) {
            // 用户主动停止，不报告异常
            appendLog("PROCESS", QString("进程已停止，退出码: %1").arg(exitCode));
            ElaMessageBar::information(ElaMessageBarType::BottomRight, "信息",
                                       "进程已停止", 1500);
        } else if (exitStatus == QProcess::NormalExit) {
            // 进程正常退出
            appendLog("PROCESS", QString("进程正常退出，退出码: %1").arg(exitCode));
            ElaMessageBar::information(ElaMessageBarType::BottomRight, "信息",
                                       QString("进程已退出 (代码: %1)").arg(exitCode), 2000);
        } else {
            // 进程异常崩溃
            appendLog("ERROR", "进程异常崩溃");
            ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                                 "进程异常崩溃", 2000);
        }

        emit processStopped(_currentProjectName, exitCode);
    });

    // 进程错误信号
    connect(_processRunner.get(), &ProcessRunner::errorOccurred, this, [this](const QString& error) {
        appendLog("ERROR", error);
        ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误", error, 2000);
        emit processError(_currentProjectName, error);
    });

    // 进程状态变化信号
    connect(_processRunner.get(), &ProcessRunner::stateChanged, this,
            [this](ProcessRunner::ProcessState state) {
        bool isRunning = (state == ProcessRunner::ProcessState::Running);
        updateProcessStatus(isRunning);

        // 记录状态变化日志
        QString stateStr;
        switch (state) {
        case ProcessRunner::ProcessState::NotStarted:
            stateStr = "未启动";
            break;
        case ProcessRunner::ProcessState::Running:
            stateStr = "运行中";
            break;
        case ProcessRunner::ProcessState::Finished:
            stateStr = "已完成";
            break;
        case ProcessRunner::ProcessState::Error:
            stateStr = "错误";
            break;
        case ProcessRunner::ProcessState::Killed:
            stateStr = "已终止";
            break;
        }
        qDebug() << "ProcessPage 进程状态变化:" << stateStr;
    });
}

void ProcessPage::setProjectName(const QString& projectName)
{
    _currentProjectName = projectName;
    qDebug() << "ProcessPage 设置项目名称:" << projectName;

    // 重新加载该项目的配置
    loadProcessConfig();
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

    if (_processRunner->isRunning()) {
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

    appendLog("PROCESS", QString("启动程序: %1").arg(program));
    if (!arguments.isEmpty()) {
        appendLog("PROCESS", QString("命令参数: %1").arg(arguments.join(" ")));
    }

    // 使用 ProcessRunner 启动进程
    if (!_processRunner->start(program, arguments)) {
        appendLog("ERROR", "进程启动失败");
        ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                             "进程启动失败", 2000);
    }
}

void ProcessPage::stopProcess(bool forceKill)
{
    if (!_processRunner->isRunning()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "进程未运行", 1500);
        return;
    }

    appendLog("PROCESS", forceKill ? "正在强制终止进程..." : "正在终止进程...");

    // 使用 ProcessRunner 停止进程
    _processRunner->stop(forceKill);

    // 如果是优雅终止，等待一段时间后检查是否需要强制杀死
    if (!forceKill) {
        QTimer::singleShot(3000, this, [this]() {
            if (_processRunner->isRunning()) {
                appendLog("WARNING", "进程未响应，强制终止");
                _processRunner->stop(true);  // 强制杀死
            }
        });
    }
}

void ProcessPage::clearLog()
{
    _logTextEdit->clear();
    appendLog("INFO", "日志已清空");
}

void ProcessPage::appendLog(const QString& level, const QString& message)
{
    if (!_logTextEdit) {
        return;
    }

    // 获取当前时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 根据当前主题选择颜色方案
    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;
    QMap<QString, QString> &colors = isDark ? m_darkColors : m_lightColors;

    // 获取颜色
    QString color = colors.value(level, isDark ? "#D4D4D4" : "#323130");

    // 构建带颜色的日志行
    QString logLine = QString("<span style='color: %1;'>[%2] [%3] %4</span>")
                          .arg(color)
                          .arg(timestamp)
                          .arg(level)
                          .arg(message);

    // 追加日志（使用HTML格式）
    _logTextEdit->appendHtml(logLine);

    // 自动滚动到底部
    QTextCursor cursor = _logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    _logTextEdit->setTextCursor(cursor);
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

void ProcessPage::onThemeChanged(ElaThemeType::ThemeMode mode)
{
    if (!_logTextEdit) {
        return;
    }

    // 根据主题更新日志窗口样式
    bool isDark = (mode == ElaThemeType::Dark);

    if (isDark) {
        // 深色主题：深色背景 + 浅色文字
        _logTextEdit->setStyleSheet(
            "ElaPlainTextEdit { "
            "background-color: #1E1E1E; "
            "color: #D4D4D4; "
            "font-family: 'Consolas', 'Courier New', monospace; "
            "font-size: 11pt; "
            "}"
        );
    } else {
        // 浅色主题：浅色背景 + 深色文字
        _logTextEdit->setStyleSheet(
            "ElaPlainTextEdit { "
            "background-color: #FFFFFF; "
            "color: #323130; "
            "font-family: 'Consolas', 'Courier New', monospace; "
            "font-size: 11pt; "
            "border: 1px solid #E1E1E1; "
            "}"
        );
    }

    // 记录主题切换日志
    QString themeName = isDark ? "Dark" : "Light";
    appendLog("DEBUG", QString("主题已切换: %1").arg(themeName));
}

void ProcessPage::browseProgram()
{
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择可执行程序",
        QString(),  // 默认目录
#ifdef Q_OS_WIN
        "可执行文件 (*.exe *.bat *.cmd);;所有文件 (*.*)"
#else
        "可执行文件 (*);;所有文件 (*.*)"
#endif
    );

    if (!fileName.isEmpty()) {
        _programLineEdit->setText(fileName);
        appendLog("INFO", QString("已选择程序: %1").arg(fileName));
    }
}

void ProcessPage::saveProcessConfig()
{
    if (!_programLineEdit || !_argumentsLineEdit) {
        return;
    }

    // 使用项目名称作为配置键的一部分
    QString settingsKey = QString("ProcessPage/%1").arg(_currentProjectName.isEmpty() ? "default" : _currentProjectName);

    QSettings settings("GTTC", "Prism");
    settings.beginGroup(settingsKey);
    settings.setValue("program", _programLineEdit->text());
    settings.setValue("arguments", _argumentsLineEdit->text());
    settings.endGroup();

    qDebug() << "ProcessPage 保存配置:" << settingsKey
             << "program:" << _programLineEdit->text()
             << "arguments:" << _argumentsLineEdit->text();
}

void ProcessPage::loadProcessConfig()
{
    if (!_programLineEdit || !_argumentsLineEdit) {
        return;
    }

    // 使用项目名称作为配置键的一部分
    QString settingsKey = QString("ProcessPage/%1").arg(_currentProjectName.isEmpty() ? "default" : _currentProjectName);

    QSettings settings("GTTC", "Prism");
    settings.beginGroup(settingsKey);
    QString program = settings.value("program", "").toString();
    QString arguments = settings.value("arguments", "").toString();
    settings.endGroup();

    // 阻止信号触发，避免重复保存
    _programLineEdit->blockSignals(true);
    _argumentsLineEdit->blockSignals(true);

    _programLineEdit->setText(program);
    _argumentsLineEdit->setText(arguments);

    _programLineEdit->blockSignals(false);
    _argumentsLineEdit->blockSignals(false);

    if (!program.isEmpty()) {
        appendLog("INFO", QString("已加载上次配置: %1").arg(program));
        qDebug() << "ProcessPage 加载配置:" << settingsKey
                 << "program:" << program
                 << "arguments:" << arguments;
    }
}

} // namespace Prism
