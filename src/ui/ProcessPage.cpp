#include "ProcessPage.h"
#include "ProcessEditDialog.h"
#include "core/ProcessRunner.h"
#include "components/LogToolBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QDateTime>
#include <QTextCursor>
#include <QTextBlock>
#include <QDebug>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>
#include <QFileDialog>
#include <QTextStream>

#include "ElaPlainTextEdit.h"
#include "ElaPushButton.h"
#include "ElaText.h"
#include "ElaMessageBar.h"
#include "ElaTheme.h"
#include "ElaTabWidget.h"
#include "ElaTableView.h"

namespace Prism {

// 表格列索引
enum TableColumn {
    COL_STATUS = 0,
    COL_NAME,
    COL_PROGRAM,
    COL_OPERATION,
    COL_COUNT
};

ProcessPage::ProcessPage(QWidget* parent)
    : BasePage(parent)
{
    setWindowTitle("进程监控");

    // 定义日志级别对应的颜色（深色主题）
    m_darkColors["INFO"] = "#61AFEF";
    m_darkColors["SUCCESS"] = "#98C379";
    m_darkColors["WARNING"] = "#E5C07B";
    m_darkColors["ERROR"] = "#E06C75";
    m_darkColors["DEBUG"] = "#C678DD";
    m_darkColors["PROCESS"] = "#56B6C2";
    m_darkColors["STDOUT"] = "#ABB2BF";
    m_darkColors["STDERR"] = "#E06C75";

    // 定义日志级别对应的颜色（浅色主题）
    m_lightColors["INFO"] = "#0078D4";
    m_lightColors["SUCCESS"] = "#107C10";
    m_lightColors["WARNING"] = "#F7630C";
    m_lightColors["ERROR"] = "#D13438";
    m_lightColors["DEBUG"] = "#8764B8";
    m_lightColors["PROCESS"] = "#00979C";
    m_lightColors["STDOUT"] = "#323130";
    m_lightColors["STDERR"] = "#D13438";

    initUI();
    setupConnections();

    // 连接主题切换信号
    connect(eTheme, &ElaTheme::themeModeChanged, this, &ProcessPage::onThemeChanged);

    // 加载持久化配置
    loadProcessConfig();

    qDebug() << "ProcessPage 表格式布局初始化完成";
}

ProcessPage::~ProcessPage()
{
    // 停止所有运行中的进程
    for (auto& data : _processData) {
        if (data.runner && data.runner->isRunning()) {
            data.runner->stop(true);
            data.runner->waitForFinished(3000);
        }
    }
}

void ProcessPage::initUI()
{
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("进程监控");
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // ========================================
    // 顶部：工具栏
    // ========================================
    QWidget* toolbarWidget = new QWidget(centralWidget);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(10);

    _addProcessButton = new ElaPushButton("+ 添加进程", toolbarWidget);
    _addProcessButton->setFixedWidth(100);

    _startAllButton = new ElaPushButton("▶ 全部启动", toolbarWidget);
    _startAllButton->setFixedWidth(100);

    _stopAllButton = new ElaPushButton("■ 全部停止", toolbarWidget);
    _stopAllButton->setFixedWidth(100);
    _stopAllButton->setEnabled(false);

    toolbarLayout->addWidget(_addProcessButton);
    toolbarLayout->addWidget(_startAllButton);
    toolbarLayout->addWidget(_stopAllButton);
    toolbarLayout->addStretch();

    mainLayout->addWidget(toolbarWidget);

    // ========================================
    // 中间：进程表格
    // ========================================
    _processTable = new ElaTableView(centralWidget);
    _processTableModel = new QStandardItemModel(0, COL_COUNT, this);
    _processTableModel->setHorizontalHeaderLabels({"状态", "进程名称", "程序路径", "操作"});

    _processTable->setModel(_processTableModel);
    _processTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    _processTable->setSelectionMode(QAbstractItemView::SingleSelection);
    _processTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _processTable->setAlternatingRowColors(true);
    _processTable->verticalHeader()->setVisible(false);
    _processTable->setShowGrid(true);

    // 设置列宽
    QHeaderView* header = _processTable->horizontalHeader();
    header->setSectionResizeMode(COL_STATUS, QHeaderView::Fixed);
    header->setSectionResizeMode(COL_NAME, QHeaderView::Interactive);
    header->setSectionResizeMode(COL_PROGRAM, QHeaderView::Stretch);
    header->setSectionResizeMode(COL_OPERATION, QHeaderView::Fixed);
    _processTable->setColumnWidth(COL_STATUS, 60);
    _processTable->setColumnWidth(COL_NAME, 150);
    _processTable->setColumnWidth(COL_OPERATION, 200);

    _processTable->setMinimumHeight(120);
    _processTable->setMaximumHeight(200);

    mainLayout->addWidget(_processTable);

    // ========================================
    // 底部：日志 Tab 区域
    // ========================================
    ElaText* logTitle = new ElaText("日志输出", centralWidget);
    logTitle->setTextPixelSize(16);
    mainLayout->addWidget(logTitle);

    // 日志工具栏
    _logToolBar = new LogToolBar(centralWidget);
    mainLayout->addWidget(_logToolBar);

    // 初始化过滤级别
    _activeFilterLevels = _logToolBar->getFilterLevels();

    _logTabWidget = new ElaTabWidget(centralWidget);
    _logTabWidget->setTabsClosable(false);
    _logTabWidget->setMovable(false);

    // 创建"全部"日志页
    _allLogTextEdit = createLogTextEdit();
    _logTabWidget->addTab(_allLogTextEdit, "全部");

    mainLayout->addWidget(_logTabWidget, 1);

    // ========================================
    // 底部按钮区域
    // ========================================
    QWidget* bottomWidget = new QWidget(centralWidget);
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->addStretch();

    _clearCurrentLogButton = new ElaPushButton("清空当前日志", bottomWidget);
    _clearCurrentLogButton->setFixedWidth(120);

    _clearAllLogButton = new ElaPushButton("清空全部日志", bottomWidget);
    _clearAllLogButton->setFixedWidth(120);

    bottomLayout->addWidget(_clearCurrentLogButton);
    bottomLayout->addWidget(_clearAllLogButton);

    mainLayout->addWidget(bottomWidget);

    addCentralWidget(centralWidget, true, false, 0);

    // 初始日志
    appendLog(_allLogTextEdit, "INFO", "进程监控页面已就绪");
    appendLog(_allLogTextEdit, "INFO", "点击「+ 添加进程」按钮配置新的进程");
}

void ProcessPage::setupConnections()
{
    connect(_addProcessButton, &ElaPushButton::clicked, this, &ProcessPage::addProcess);
    connect(_startAllButton, &ElaPushButton::clicked, this, &ProcessPage::startAllProcesses);
    connect(_stopAllButton, &ElaPushButton::clicked, this, &ProcessPage::stopAllProcesses);
    connect(_clearCurrentLogButton, &ElaPushButton::clicked, this, &ProcessPage::clearCurrentLog);
    connect(_clearAllLogButton, &ElaPushButton::clicked, this, &ProcessPage::clearAllLogs);

    // 日志工具栏连接
    connect(_logToolBar, &LogToolBar::filterChanged, this, &ProcessPage::setLogFilter);
    connect(_logToolBar, &LogToolBar::searchRequested, this, &ProcessPage::searchLogs);
    connect(_logToolBar, &LogToolBar::exportRequested, this, &ProcessPage::exportLogs);
    connect(_logToolBar, &LogToolBar::clearRequested, this, &ProcessPage::clearCurrentLog);
}

void ProcessPage::setProjectName(const QString& projectName)
{
    if (_currentProjectName == projectName) {
        return;
    }

    // 停止所有进程
    stopAllProcesses();

    // 清理表格和数据
    _processTableModel->setRowCount(0);
    for (const auto& data : _processData) {
        removeLogTab(data.config.id);
    }
    _processData.clear();

    _currentProjectName = projectName;
    loadProcessConfig();
}

void ProcessPage::addProcess()
{
    ProcessEditDialog dialog(this, false);
    if (dialog.exec() == QDialog::Accepted) {
        ProcessConfig config = dialog.getConfig();
        addTableRow(config);
        addLogTab(config.id, config.name);
        saveProcessConfig();

        appendLog(_allLogTextEdit, "INFO", QString("已添加进程: %1").arg(config.name));
        updateGlobalButtonStates();
    }
}

void ProcessPage::editProcess(int row)
{
    if (row < 0 || row >= _processData.size()) {
        return;
    }

    // 运行中不允许编辑
    auto status = _processData[row].currentStatus;
    if (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "请先停止进程再编辑", 1500);
        return;
    }

    ProcessEditDialog dialog(this, true);
    dialog.setConfig(_processData[row].config);

    if (dialog.exec() == QDialog::Accepted) {
        ProcessConfig newConfig = dialog.getConfig();
        QString oldName = _processData[row].config.name;

        _processData[row].config = newConfig;
        updateTableRow(row);
        updateLogTabName(newConfig.id, newConfig.name);
        saveProcessConfig();

        appendLog(_allLogTextEdit, "INFO", QString("已更新进程: %1 -> %2").arg(oldName).arg(newConfig.name));
    }
}

void ProcessPage::deleteProcess(int row)
{
    if (row < 0 || row >= _processData.size()) {
        return;
    }

    // 运行中先停止
    auto status = _processData[row].currentStatus;
    if (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting) {
        _processData[row].runner->stop(true);
        _processData[row].runner->waitForFinished(3000);
    }

    QString name = _processData[row].config.name;
    QString configId = _processData[row].config.id;

    removeLogTab(configId);
    removeTableRow(row);
    saveProcessConfig();

    appendLog(_allLogTextEdit, "INFO", QString("已删除进程: %1").arg(name));
    updateGlobalButtonStates();
}

void ProcessPage::startProcess(int row)
{
    if (row < 0 || row >= _processData.size()) {
        return;
    }

    auto& data = _processData[row];
    auto status = data.currentStatus;
    if (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting) {
        return;
    }

    if (data.config.program.isEmpty()) {
        ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                               "未设置程序路径", 1500);
        return;
    }

    // 创建 ProcessRunner（如果不存在）
    if (!data.runner) {
        data.runner = std::make_shared<ProcessRunner>(this);
        connectProcessSignals(row);
    }

    // 设置为启动中状态
    data.currentStatus = ProcessStatusIndicator::Status::Starting;
    updateTableRow(row);
    updateGlobalButtonStates();

    QStringList arguments;
    if (!data.config.arguments.isEmpty()) {
        arguments = data.config.arguments.split(' ', QString::SkipEmptyParts);
    }

    appendLogToTab(data.config.id, "PROCESS", QString("启动程序: %1").arg(data.config.program));
    if (data.config.showConsole) {
        appendLogToTab(data.config.id, "INFO", "已启用独立控制台窗口，日志将不会在此处显示");
    }

    if (!data.runner->start(data.config.program, arguments, data.config.workingDirectory, data.config.showConsole)) {
        // 启动失败，恢复为错误状态
        data.currentStatus = ProcessStatusIndicator::Status::Error;
        updateTableRow(row);
        updateGlobalButtonStates();
        appendLogToTab(data.config.id, "ERROR", "进程启动失败");
    }
}

void ProcessPage::stopProcess(int row)
{
    if (row < 0 || row >= _processData.size()) {
        return;
    }

    auto& data = _processData[row];
    auto status = data.currentStatus;
    bool isRunning = (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting);
    if (!isRunning || !data.runner) {
        return;
    }

    appendLogToTab(data.config.id, "PROCESS", "正在停止进程...");
    data.runner->stop(false);
}

void ProcessPage::startAllProcesses()
{
    int count = 0;
    for (int i = 0; i < _processData.size(); ++i) {
        auto status = _processData[i].currentStatus;
        bool isRunning = (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting);
        if (!isRunning && !_processData[i].config.program.isEmpty()) {
            startProcess(i);
            count++;
        }
    }

    if (count > 0) {
        ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                               QString("正在启动 %1 个进程").arg(count), 1500);
    }
}

void ProcessPage::stopAllProcesses()
{
    int count = 0;
    for (int i = 0; i < _processData.size(); ++i) {
        auto status = _processData[i].currentStatus;
        bool isRunning = (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting);
        if (isRunning) {
            stopProcess(i);
            count++;
        }
    }

    if (count > 0) {
        appendLog(_allLogTextEdit, "INFO", QString("正在停止 %1 个进程").arg(count));
    }
}

void ProcessPage::clearCurrentLog()
{
    int currentIndex = _logTabWidget->currentIndex();
    if (currentIndex == 0) {
        _allLogTextEdit->clear();
        appendLog(_allLogTextEdit, "INFO", "日志已清空");
    } else {
        QWidget* widget = _logTabWidget->widget(currentIndex);
        ElaPlainTextEdit* textEdit = qobject_cast<ElaPlainTextEdit*>(widget);
        if (textEdit) {
            textEdit->clear();
            appendLog(textEdit, "INFO", "日志已清空");
        }
    }
}

void ProcessPage::clearAllLogs()
{
    _allLogTextEdit->clear();
    appendLog(_allLogTextEdit, "INFO", "所有日志已清空");

    for (auto it = _logTextEdits.begin(); it != _logTextEdits.end(); ++it) {
        it.value()->clear();
    }
}

void ProcessPage::onThemeChanged(ElaThemeType::ThemeMode mode)
{
    // 更新日志样式
    updateLogStyle(_allLogTextEdit);
    for (auto it = _logTextEdits.begin(); it != _logTextEdits.end(); ++it) {
        updateLogStyle(it.value());
    }
}

void ProcessPage::addTableRow(const ProcessConfig& config)
{
    int row = _processTableModel->rowCount();

    // 添加数据
    ProcessRowData rowData;
    rowData.config = config;
    rowData.currentStatus = ProcessStatusIndicator::Status::Stopped;
    _processData.append(rowData);

    // 创建行数据
    QList<QStandardItem*> items;

    // 状态列（占位，使用 widget 显示）
    QStandardItem* statusItem = new QStandardItem();
    statusItem->setTextAlignment(Qt::AlignCenter);
    items.append(statusItem);

    // 名称列
    QStandardItem* nameItem = new QStandardItem(config.name);
    items.append(nameItem);

    // 程序路径列
    QStandardItem* programItem = new QStandardItem(config.program);
    programItem->setToolTip(config.program);
    items.append(programItem);

    // 操作列（占位，实际使用 widget）
    QStandardItem* operationItem = new QStandardItem();
    items.append(operationItem);

    _processTableModel->appendRow(items);

    // 设置状态列的指示灯 widget（居中显示）
    QWidget* statusContainer = new QWidget();
    QHBoxLayout* statusLayout = new QHBoxLayout(statusContainer);
    statusLayout->setContentsMargins(0, 0, 0, 0);
    statusLayout->setAlignment(Qt::AlignCenter);
    ProcessStatusIndicator* indicator = new ProcessStatusIndicator(statusContainer);
    indicator->setStatus(ProcessStatusIndicator::Status::Stopped);
    indicator->setToolTip(ProcessStatusIndicator::statusName(ProcessStatusIndicator::Status::Stopped));
    statusLayout->addWidget(indicator);
    _processTable->setIndexWidget(_processTableModel->index(row, COL_STATUS), statusContainer);

    // 设置操作列的 widget
    QWidget* opWidget = createOperationWidget(row);
    _processTable->setIndexWidget(_processTableModel->index(row, COL_OPERATION), opWidget);

    _processTable->setRowHeight(row, 36);
}

void ProcessPage::updateTableRow(int row)
{
    if (row < 0 || row >= _processData.size()) {
        return;
    }

    const auto& data = _processData[row];

    // 更新状态指示灯
    updateStatusIndicator(row, data.currentStatus);

    // 更新名称
    QStandardItem* nameItem = _processTableModel->item(row, COL_NAME);
    if (nameItem) {
        nameItem->setText(data.config.name);
    }

    // 更新程序路径
    QStandardItem* programItem = _processTableModel->item(row, COL_PROGRAM);
    if (programItem) {
        programItem->setText(data.config.program);
        programItem->setToolTip(data.config.program);
    }

    // 更新操作按钮状态
    updateOperationButtons(row, data.currentStatus);
}

void ProcessPage::removeTableRow(int row)
{
    if (row < 0 || row >= _processData.size()) {
        return;
    }

    _processTableModel->removeRow(row);
    _processData.removeAt(row);

    // 更新后续行的 widget（因为 row 索引变了）
    for (int i = row; i < _processData.size(); ++i) {
        // 重建状态指示灯（居中显示）
        QWidget* statusContainer = new QWidget();
        QHBoxLayout* statusLayout = new QHBoxLayout(statusContainer);
        statusLayout->setContentsMargins(0, 0, 0, 0);
        statusLayout->setAlignment(Qt::AlignCenter);
        ProcessStatusIndicator* indicator = new ProcessStatusIndicator(statusContainer);
        indicator->setStatus(_processData[i].currentStatus);
        indicator->setToolTip(ProcessStatusIndicator::statusName(_processData[i].currentStatus));
        statusLayout->addWidget(indicator);
        _processTable->setIndexWidget(_processTableModel->index(i, COL_STATUS), statusContainer);

        // 重建操作按钮
        QWidget* opWidget = createOperationWidget(i);
        _processTable->setIndexWidget(_processTableModel->index(i, COL_OPERATION), opWidget);
    }
}

QWidget* ProcessPage::createOperationWidget(int row)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 2, 5, 2);
    layout->setSpacing(5);

    auto status = (row < _processData.size()) ? _processData[row].currentStatus : ProcessStatusIndicator::Status::Stopped;
    bool isRunning = (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting);

    ElaPushButton* startStopBtn = new ElaPushButton(isRunning ? "停止" : "启动", widget);
    startStopBtn->setFixedSize(50, 26);
    startStopBtn->setObjectName("startStopBtn");

    ElaPushButton* editBtn = new ElaPushButton("编辑", widget);
    editBtn->setFixedSize(50, 26);
    editBtn->setEnabled(!isRunning);
    editBtn->setObjectName("editBtn");

    ElaPushButton* deleteBtn = new ElaPushButton("删除", widget);
    deleteBtn->setFixedSize(50, 26);
    deleteBtn->setObjectName("deleteBtn");

    layout->addWidget(startStopBtn);
    layout->addWidget(editBtn);
    layout->addWidget(deleteBtn);
    layout->addStretch();

    // 连接信号（使用 lambda 捕获当前行）
    connect(startStopBtn, &ElaPushButton::clicked, this, [this, row]() {
        if (row < _processData.size()) {
            auto status = _processData[row].currentStatus;
            bool isRunning = (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting);
            if (isRunning) {
                stopProcess(row);
            } else {
                startProcess(row);
            }
        }
    });

    connect(editBtn, &ElaPushButton::clicked, this, [this, row]() {
        editProcess(row);
    });

    connect(deleteBtn, &ElaPushButton::clicked, this, [this, row]() {
        deleteProcess(row);
    });

    return widget;
}

void ProcessPage::updateOperationButtons(int row, ProcessStatusIndicator::Status status)
{
    QWidget* widget = _processTable->indexWidget(_processTableModel->index(row, COL_OPERATION));
    if (!widget) {
        return;
    }

    bool isRunning = (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting);

    ElaPushButton* startStopBtn = widget->findChild<ElaPushButton*>("startStopBtn");
    ElaPushButton* editBtn = widget->findChild<ElaPushButton*>("editBtn");

    if (startStopBtn) {
        startStopBtn->setText(isRunning ? "停止" : "启动");
    }
    if (editBtn) {
        editBtn->setEnabled(!isRunning);
    }
}

void ProcessPage::updateStatusIndicator(int row, ProcessStatusIndicator::Status status)
{
    QWidget* container = _processTable->indexWidget(_processTableModel->index(row, COL_STATUS));
    if (container) {
        ProcessStatusIndicator* indicator = container->findChild<ProcessStatusIndicator*>();
        if (indicator) {
            indicator->setStatus(status);
            indicator->setToolTip(ProcessStatusIndicator::statusName(status));
        }
    }
}

void ProcessPage::connectProcessSignals(int row)
{
    if (row < 0 || row >= _processData.size()) {
        return;
    }

    auto& data = _processData[row];
    if (!data.runner) {
        return;
    }

    QString configId = data.config.id;

    connect(data.runner.get(), &ProcessRunner::started, this, [this, configId]() {
        onProcessStarted(configId);
    });

    connect(data.runner.get(), &ProcessRunner::finished, this,
            [this, configId](int exitCode, QProcess::ExitStatus status) {
        onProcessFinished(configId, exitCode, status);
    });

    connect(data.runner.get(), &ProcessRunner::errorOccurred, this,
            [this, configId](const QString& error) {
        onProcessError(configId, error);
    });

    connect(data.runner.get(), &ProcessRunner::standardOutput, this,
            [this, configId](const QString& output) {
        onProcessOutput(configId, output, false);
    });

    connect(data.runner.get(), &ProcessRunner::standardError, this,
            [this, configId](const QString& error) {
        onProcessOutput(configId, error, true);
    });

    // 监听状态变化以处理 Killed 状态
    connect(data.runner.get(), &ProcessRunner::stateChanged, this,
            [this, configId](ProcessRunner::ProcessState state) {
        if (state == ProcessRunner::ProcessState::Killed) {
            int row = findRowByConfigId(configId);
            if (row >= 0) {
                _processData[row].currentStatus = ProcessStatusIndicator::Status::Killed;
                updateTableRow(row);
                updateGlobalButtonStates();
                appendLogToTab(configId, "WARNING", "进程已被强制终止");
            }
        }
    });
}

void ProcessPage::onProcessStarted(const QString& configId)
{
    int row = findRowByConfigId(configId);
    if (row < 0) return;

    _processData[row].currentStatus = ProcessStatusIndicator::Status::Running;
    updateTableRow(row);
    updateGlobalButtonStates();

    appendLogToTab(configId, "SUCCESS", "进程已启动");
    emit processStarted(_currentProjectName, _processData[row].config.name);
}

void ProcessPage::onProcessFinished(const QString& configId, int exitCode, QProcess::ExitStatus status)
{
    int row = findRowByConfigId(configId);
    if (row < 0) return;

    // 根据退出状态设置不同的指示灯颜色
    if (status == QProcess::NormalExit) {
        if (exitCode == 0) {
            // 正常退出（退出码为0）
            _processData[row].currentStatus = ProcessStatusIndicator::Status::Stopped;
            appendLogToTab(configId, "PROCESS", QString("进程正常退出，退出码: %1").arg(exitCode));
        } else {
            // 正常退出但退出码非0，视为错误
            _processData[row].currentStatus = ProcessStatusIndicator::Status::Error;
            appendLogToTab(configId, "WARNING", QString("进程退出，退出码: %1").arg(exitCode));
        }
    } else {
        // 崩溃退出
        _processData[row].currentStatus = ProcessStatusIndicator::Status::Error;
        appendLogToTab(configId, "ERROR", "进程异常崩溃");
    }

    updateTableRow(row);
    updateGlobalButtonStates();

    emit processStopped(_currentProjectName, _processData[row].config.name, exitCode);
}

void ProcessPage::onProcessError(const QString& configId, const QString& error)
{
    appendLogToTab(configId, "ERROR", error);
}

void ProcessPage::onProcessOutput(const QString& configId, const QString& output, bool isError)
{
    QStringList lines = output.split('\n', QString::SkipEmptyParts);
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            appendLogToTab(configId, isError ? "STDERR" : "STDOUT", trimmed);
        }
    }
}

ElaPlainTextEdit* ProcessPage::createLogTextEdit()
{
    ElaPlainTextEdit* textEdit = new ElaPlainTextEdit(this);
    textEdit->setReadOnly(true);
    updateLogStyle(textEdit);
    return textEdit;
}

void ProcessPage::addLogTab(const QString& configId, const QString& name)
{
    ElaPlainTextEdit* textEdit = createLogTextEdit();
    int tabIndex = _logTabWidget->addTab(textEdit, name);
    _logTextEdits[configId] = textEdit;
    _logTabIndices[configId] = tabIndex;
}

void ProcessPage::removeLogTab(const QString& configId)
{
    if (!_logTabIndices.contains(configId)) {
        return;
    }

    int tabIndex = _logTabIndices[configId];
    _logTabWidget->removeTab(tabIndex);
    _logTextEdits.remove(configId);
    _logTabIndices.remove(configId);

    // 重新计算索引
    int index = 1;
    for (auto it = _logTabIndices.begin(); it != _logTabIndices.end(); ++it) {
        it.value() = index++;
    }
}

void ProcessPage::updateLogTabName(const QString& configId, const QString& newName)
{
    if (_logTabIndices.contains(configId)) {
        int tabIndex = _logTabIndices[configId];
        _logTabWidget->setTabText(tabIndex, newName);
    }
}

void ProcessPage::appendLog(ElaPlainTextEdit* textEdit, const QString& level, const QString& message)
{
    if (!textEdit) return;

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;
    QMap<QString, QString>& colors = isDark ? m_darkColors : m_lightColors;
    QString color = colors.value(level, isDark ? "#D4D4D4" : "#323130");

    QString logLine = QString("<span style='color: %1;'>[%2] [%3] %4</span>")
                          .arg(color).arg(timestamp).arg(level).arg(message.toHtmlEscaped());

    textEdit->appendHtml(logLine);

    // 行数限制
    trimLogLines(textEdit);

    QTextCursor cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    textEdit->setTextCursor(cursor);
}

void ProcessPage::appendLogToTab(const QString& configId, const QString& level, const QString& message)
{
    // 存储日志条目
    storeLogEntry(configId, level, message);

    // 检查是否应该显示
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    entry.level = level;
    entry.configId = configId;
    entry.message = message;

    if (!shouldShowLogEntry(entry)) {
        return;
    }

    // 追加到对应进程的日志
    if (_logTextEdits.contains(configId)) {
        appendLog(_logTextEdits[configId], level, message);
    }

    // 同时追加到"全部"日志
    int row = findRowByConfigId(configId);
    QString processName = (row >= 0) ? _processData[row].config.name : "Unknown";
    QString prefixedMessage = QString("[%1] %2").arg(processName).arg(message);
    appendLog(_allLogTextEdit, level, prefixedMessage);
}

void ProcessPage::updateLogStyle(ElaPlainTextEdit* textEdit)
{
    if (!textEdit) return;

    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;
    if (isDark) {
        textEdit->setStyleSheet(
            "ElaPlainTextEdit { "
            "background-color: #1E1E1E; "
            "color: #D4D4D4; "
            "font-family: 'Consolas', 'Courier New', monospace; "
            "font-size: 11pt; "
            "border: 1px solid #3D3D3D; "
            "}"
        );
    } else {
        textEdit->setStyleSheet(
            "ElaPlainTextEdit { "
            "background-color: #FFFFFF; "
            "color: #323130; "
            "font-family: 'Consolas', 'Courier New', monospace; "
            "font-size: 11pt; "
            "border: 1px solid #E1E1E1; "
            "}"
        );
    }
}

void ProcessPage::saveProcessConfig()
{
    QString settingsKey = QString("ProcessPage/MultiProcess/%1")
                              .arg(_currentProjectName.isEmpty() ? "default" : _currentProjectName);

    QSettings settings("GTTC", "Prism");

    QJsonArray configArray;
    for (const auto& data : _processData) {
        QJsonObject obj;
        obj["id"] = data.config.id;
        obj["name"] = data.config.name;
        obj["program"] = data.config.program;
        obj["arguments"] = data.config.arguments;
        obj["workingDirectory"] = data.config.workingDirectory;
        obj["showConsole"] = data.config.showConsole;
        configArray.append(obj);
    }

    QJsonDocument doc(configArray);
    settings.setValue(settingsKey, doc.toJson(QJsonDocument::Compact));
}

void ProcessPage::loadProcessConfig()
{
    QString settingsKey = QString("ProcessPage/MultiProcess/%1")
                              .arg(_currentProjectName.isEmpty() ? "default" : _currentProjectName);

    QSettings settings("GTTC", "Prism");
    QString jsonStr = settings.value(settingsKey, "").toString();

    if (jsonStr.isEmpty()) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (!doc.isArray()) {
        return;
    }

    QJsonArray configArray = doc.array();
    for (const QJsonValue& value : configArray) {
        QJsonObject obj = value.toObject();

        ProcessConfig config;
        config.id = obj["id"].toString();
        config.name = obj["name"].toString();
        config.program = obj["program"].toString();
        config.arguments = obj["arguments"].toString();
        config.workingDirectory = obj["workingDirectory"].toString();
        config.showConsole = obj["showConsole"].toBool(false);

        if (config.id.isEmpty()) {
            config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        }

        addTableRow(config);
        addLogTab(config.id, config.name);
    }

    if (!_processData.isEmpty()) {
        appendLog(_allLogTextEdit, "INFO",
                  QString("已加载 %1 个进程配置").arg(_processData.size()));
    }

    updateGlobalButtonStates();
}

int ProcessPage::findRowByConfigId(const QString& configId)
{
    for (int i = 0; i < _processData.size(); ++i) {
        if (_processData[i].config.id == configId) {
            return i;
        }
    }
    return -1;
}

void ProcessPage::updateGlobalButtonStates()
{
    bool hasRunning = false;
    bool hasConfigured = false;

    for (const auto& data : _processData) {
        auto status = data.currentStatus;
        if (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting) {
            hasRunning = true;
        }
        if (!data.config.program.isEmpty()) {
            hasConfigured = true;
        }
    }

    _startAllButton->setEnabled(hasConfigured && !hasRunning);
    _stopAllButton->setEnabled(hasRunning);
}

int ProcessPage::getRunningProcessCount() const
{
    int count = 0;
    for (const auto& data : _processData) {
        auto status = data.currentStatus;
        if (status == ProcessStatusIndicator::Status::Running || status == ProcessStatusIndicator::Status::Starting) {
            count++;
        }
    }
    return count;
}

// ========================================
// 日志增强方法
// ========================================

void ProcessPage::trimLogLines(ElaPlainTextEdit* textEdit)
{
    if (!textEdit) return;

    QTextDocument* doc = textEdit->document();
    while (doc->blockCount() > MAX_LOG_LINES) {
        QTextCursor cursor(doc->begin());
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar();  // 删除换行符
    }
}

void ProcessPage::storeLogEntry(const QString& configId, const QString& level, const QString& message)
{
    LogEntry entry;
    entry.timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    entry.level = level;
    entry.configId = configId;
    entry.message = message;

    _logBuffer.append(entry);

    // 限制缓存大小
    while (_logBuffer.size() > MAX_LOG_LINES * 2) {
        _logBuffer.removeFirst();
    }
}

bool ProcessPage::shouldShowLogEntry(const LogEntry& entry) const
{
    return _activeFilterLevels.contains(entry.level);
}

void ProcessPage::setLogFilter(const QStringList& levels)
{
    _activeFilterLevels = levels;
    refreshLogDisplay();
}

void ProcessPage::searchLogs(const QString& keyword)
{
    _searchKeyword = keyword;

    if (keyword.isEmpty()) {
        // 清除所有高亮
        clearSearchHighlight(_allLogTextEdit);
        for (auto* textEdit : _logTextEdits.values()) {
            clearSearchHighlight(textEdit);
        }
    } else {
        // 高亮搜索结果
        highlightSearchResults(_allLogTextEdit, keyword);
        for (auto* textEdit : _logTextEdits.values()) {
            highlightSearchResults(textEdit, keyword);
        }
    }
}

void ProcessPage::exportLogs()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出日志",
        QString("%1_logs_%2.txt")
            .arg(_currentProjectName)
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "文本文件 (*.txt);;所有文件 (*.*)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                             QString("无法创建文件: %1").arg(fileName), 2000);
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // 导出缓存的日志
    for (const LogEntry& entry : _logBuffer) {
        QString line = QString("[%1] [%2] [%3] %4")
                           .arg(entry.timestamp)
                           .arg(entry.level)
                           .arg(entry.configId.isEmpty() ? "SYSTEM" : entry.configId)
                           .arg(entry.message);
        out << line << "\n";
    }

    file.close();

    ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                           QString("日志已导出: %1").arg(fileName), 2000);
}

void ProcessPage::refreshLogDisplay()
{
    // 清空当前显示
    _allLogTextEdit->clear();
    for (auto* textEdit : _logTextEdits.values()) {
        textEdit->clear();
    }

    // 根据过滤条件重新显示日志
    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;
    QMap<QString, QString>& colors = isDark ? m_darkColors : m_lightColors;

    for (const LogEntry& entry : _logBuffer) {
        if (!shouldShowLogEntry(entry)) {
            continue;
        }

        QString color = colors.value(entry.level, isDark ? "#D4D4D4" : "#323130");

        // 追加到对应进程的日志
        if (_logTextEdits.contains(entry.configId)) {
            QString logLine = QString("<span style='color: %1;'>[%2] [%3] %4</span>")
                                  .arg(color).arg(entry.timestamp).arg(entry.level)
                                  .arg(entry.message.toHtmlEscaped());
            _logTextEdits[entry.configId]->appendHtml(logLine);
        }

        // 同时追加到"全部"日志
        int row = findRowByConfigId(entry.configId);
        QString processName = (row >= 0) ? _processData[row].config.name : "System";
        QString prefixedMessage = QString("[%1] %2").arg(processName).arg(entry.message);
        QString allLogLine = QString("<span style='color: %1;'>[%2] [%3] %4</span>")
                                 .arg(color).arg(entry.timestamp).arg(entry.level)
                                 .arg(prefixedMessage.toHtmlEscaped());
        _allLogTextEdit->appendHtml(allLogLine);
    }

    // 滚动到底部
    QTextCursor cursor = _allLogTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    _allLogTextEdit->setTextCursor(cursor);

    // 如果有搜索关键字，重新高亮
    if (!_searchKeyword.isEmpty()) {
        highlightSearchResults(_allLogTextEdit, _searchKeyword);
        for (auto* textEdit : _logTextEdits.values()) {
            highlightSearchResults(textEdit, _searchKeyword);
        }
    }
}

void ProcessPage::highlightSearchResults(ElaPlainTextEdit* textEdit, const QString& keyword)
{
    if (!textEdit || keyword.isEmpty()) return;

    // 先清除之前的高亮
    clearSearchHighlight(textEdit);

    QTextDocument* doc = textEdit->document();
    QTextCursor cursor(doc);

    // 高亮格式
    QTextCharFormat highlightFormat;
    highlightFormat.setBackground(QColor("#FFFF00"));  // 黄色背景
    highlightFormat.setForeground(QColor("#000000"));  // 黑色文字

    // 查找所有匹配
    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = doc->find(keyword, cursor, QTextDocument::FindCaseSensitively);
        if (!cursor.isNull()) {
            cursor.mergeCharFormat(highlightFormat);
        }
    }

    // 滚动到第一个匹配
    QTextCursor firstMatch = doc->find(keyword);
    if (!firstMatch.isNull()) {
        textEdit->setTextCursor(firstMatch);
        textEdit->ensureCursorVisible();
    }
}

void ProcessPage::clearSearchHighlight(ElaPlainTextEdit* textEdit)
{
    if (!textEdit) return;

    // 选择全部文本并清除格式
    QTextCursor cursor = textEdit->textCursor();
    cursor.select(QTextCursor::Document);

    QTextCharFormat defaultFormat;
    // 不设置背景颜色，保持原有的 HTML 格式
    cursor.setCharFormat(defaultFormat);
    cursor.clearSelection();

    textEdit->setTextCursor(cursor);
}

} // namespace Prism
