#include "MainWindow.h"
#include "ProjectManager.h"
#include "ProcessRunner.h"

// 标准 Qt 组件（用作占位符）
#include <QTreeView>
#include <QTabWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDebug>

// TODO: 替换为 ElaWidgetTools 组件
// #include "ElaWindow.h"
// #include "ElaTreeView.h"
// #include "ElaTabWidget.h"
// #include "ElaTextEdit.h"

namespace Prism {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_mainWindow(nullptr)
    , m_logVisible(true)
    , m_projectManager(std::make_unique<ProjectManager>(this))
{
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupConnections();

    // 设置窗口属性
    setWindowTitle("Prism - Configuration Integration Environment");
    resize(1400, 900);

    statusBar()->showMessage("Ready", 3000);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    // 创建中心部件
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 主分割器（左右）
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // 左侧：项目树
    m_projectTreeView = reinterpret_cast<ElaTreeView*>(new QTreeView(this));
    m_projectTreeModel = new QStandardItemModel(this);
    m_projectTreeModel->setHorizontalHeaderLabels({"Projects"});
    reinterpret_cast<QTreeView*>(m_projectTreeView)->setModel(m_projectTreeModel);
    reinterpret_cast<QTreeView*>(m_projectTreeView)->setMinimumWidth(200);
    reinterpret_cast<QTreeView*>(m_projectTreeView)->setMaximumWidth(400);

    // 垂直分割器（中间编辑区 + 底部日志）
    m_verticalSplitter = new QSplitter(Qt::Vertical, this);

    // 中间：标签页编辑器
    m_tabWidget = reinterpret_cast<ElaTabWidget*>(new QTabWidget(this));
    reinterpret_cast<QTabWidget*>(m_tabWidget)->setTabsClosable(true);
    reinterpret_cast<QTabWidget*>(m_tabWidget)->setMovable(true);

    // 底部：日志输出
    m_logOutput = reinterpret_cast<ElaTextEdit*>(new QTextEdit(this));
    reinterpret_cast<QTextEdit*>(m_logOutput)->setReadOnly(true);
    reinterpret_cast<QTextEdit*>(m_logOutput)->setMaximumHeight(250);
    reinterpret_cast<QTextEdit*>(m_logOutput)->setStyleSheet(
        "QTextEdit { background-color: #1E1E1E; color: #D4D4D4; font-family: 'Consolas', monospace; }"
    );

    // 组装垂直分割器
    m_verticalSplitter->addWidget(reinterpret_cast<QTabWidget*>(m_tabWidget));
    m_verticalSplitter->addWidget(reinterpret_cast<QTextEdit*>(m_logOutput));
    m_verticalSplitter->setStretchFactor(0, 3);
    m_verticalSplitter->setStretchFactor(1, 1);

    // 组装主分割器
    m_mainSplitter->addWidget(reinterpret_cast<QTreeView*>(m_projectTreeView));
    m_mainSplitter->addWidget(m_verticalSplitter);
    m_mainSplitter->setStretchFactor(0, 1);
    m_mainSplitter->setStretchFactor(1, 4);

    mainLayout->addWidget(m_mainSplitter);
    setCentralWidget(centralWidget);

    appendLog("Prism Manager initialized.", false);
}

void MainWindow::setupMenuBar() {
    // 文件菜单
    QMenu* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction("New Project", this, &MainWindow::onNewProject, QKeySequence::New);
    fileMenu->addAction("Open Config", this, &MainWindow::onOpenConfig, QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QMainWindow::close, QKeySequence::Quit);

    // 项目菜单
    QMenu* projectMenu = menuBar()->addMenu("Project");
    projectMenu->addAction("Run", this, &MainWindow::onRunProject, QKeySequence("F5"));
    projectMenu->addAction("Stop", this, &MainWindow::onStopProject, QKeySequence("Shift+F5"));

    // 帮助菜单
    QMenu* helpMenu = menuBar()->addMenu("Help");
    helpMenu->addAction("About", this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar() {
    QToolBar* toolbar = addToolBar("Main Toolbar");
    toolbar->addAction("New Project", this, &MainWindow::onNewProject);
    toolbar->addAction("Open Config", this, &MainWindow::onOpenConfig);
    toolbar->addSeparator();
    toolbar->addAction("Run (F5)", this, &MainWindow::onRunProject);
    toolbar->addAction("Stop", this, &MainWindow::onStopProject);
}

void MainWindow::setupConnections() {
    // 项目树选择事件
    connect(reinterpret_cast<QTreeView*>(m_projectTreeView)->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current, const QModelIndex&) {
        if (current.isValid()) {
            QString projectName = m_projectTreeModel->itemFromIndex(current)->text();
            onProjectSelected(projectName);
        }
    });

    // 项目管理器信号
    connect(m_projectManager.get(), &ProjectManager::projectCreated,
            this, [this](const QString& name) {
        appendLog(QString("Project created: %1").arg(name), false);
        updateProjectTree();
    });

    connect(m_projectManager.get(), &ProjectManager::projectStateChanged,
            this, [this](const QString& name, bool isRunning) {
        QString status = isRunning ? "started" : "stopped";
        appendLog(QString("Project %1 %2").arg(name).arg(status), false);
        updateProjectTree();
    });
}

void MainWindow::onProjectSelected(const QString& projectName) {
    m_currentProject = projectName;
    statusBar()->showMessage(QString("Selected project: %1").arg(projectName));
    appendLog(QString("Selected project: %1").arg(projectName), false);
}

void MainWindow::onRunProject() {
    if (m_currentProject.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a project first.");
        return;
    }

    // 示例：运行一个简单的命令（实际应从配置中读取）
    QString program = "cmd.exe";
    QStringList args = {"/c", "echo", "Running project " + m_currentProject};

    bool success = m_projectManager->runProject(m_currentProject, program, args);
    if (success) {
        // 连接进程输出
        auto runner = m_projectManager->getProcessRunner(m_currentProject);
        if (runner) {
            connect(runner.get(), &ProcessRunner::standardOutput,
                    this, &MainWindow::onProcessOutput);
            connect(runner.get(), &ProcessRunner::standardError,
                    this, &MainWindow::onProcessError);
        }
    } else {
        QMessageBox::critical(this, "Error", "Failed to start project.");
    }
}

void MainWindow::onStopProject() {
    if (m_currentProject.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a project first.");
        return;
    }

    m_projectManager->stopProject(m_currentProject);
}

void MainWindow::onOpenConfig() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Configuration File",
        QString(),
        "Config Files (*.ini *.json *.yaml *.yml *.conf *.cfg);;All Files (*)"
    );

    if (!filePath.isEmpty()) {
        appendLog(QString("Opened config: %1").arg(filePath), false);
        // TODO: 在编辑器中打开配置文件
    }
}

void MainWindow::onNewProject() {
    // 简化演示：直接弹出文件选择对话框
    QString configPath = QFileDialog::getOpenFileName(
        this,
        "Select Configuration File for New Project",
        QString(),
        "Config Files (*.ini *.json *.yaml *.yml);;All Files (*)"
    );

    if (!configPath.isEmpty()) {
        QString projectName = QFileInfo(configPath).baseName();
        bool success = m_projectManager->createProject(projectName, configPath);
        if (success) {
            m_projectManager->loadProject(projectName);
        } else {
            QMessageBox::critical(this, "Error", "Failed to create project.");
        }
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About Prism",
        "Prism - Configuration Integration Environment\n\n"
        "Version: 1.0.0\n"
        "A modern tool for radar simulation configuration management.\n\n"
        "Built with C++17, Qt 5.14+, and ElaWidgetTools.");
}

void MainWindow::onProcessOutput(const QString& output) {
    appendLog(output, false);
}

void MainWindow::onProcessError(const QString& error) {
    appendLog(error, true);
}

void MainWindow::appendLog(const QString& text, bool isError) {
    QString color = isError ? "#FF6B6B" : "#D4D4D4";
    QString html = QString("<span style='color: %1;'>%2</span>")
                       .arg(color)
                       .arg(text.toHtmlEscaped());

    reinterpret_cast<QTextEdit*>(m_logOutput)->append(html);
}

void MainWindow::updateProjectTree() {
    m_projectTreeModel->clear();
    m_projectTreeModel->setHorizontalHeaderLabels({"Projects"});

    QStringList projects = m_projectManager->getAllProjectNames();
    for (const QString& projectName : projects) {
        auto* item = new QStandardItem(projectName);

        // 获取运行状态
        auto* project = m_projectManager->getProject(projectName);
        if (project && project->isRunning) {
            item->setIcon(QIcon(":/icons/running.png")); // TODO: 添加实际图标
            item->setForeground(QColor(0, 200, 83)); // Green
        } else {
            item->setIcon(QIcon(":/icons/stopped.png")); // TODO: 添加实际图标
            item->setForeground(QColor(150, 150, 150)); // Gray
        }

        m_projectTreeModel->appendRow(item);
    }
}

} // namespace Prism
