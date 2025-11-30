#include "MainWindow.h"
#include "ProjectManager.h"
#include "HomePage.h"
#include "BasePage.h"
#include "ConfigPage.h"
#include "ProcessPage.h"

// ElaWidgetTools 组件
#include "ElaContentDialog.h"
#include "ElaDockWidget.h"
#include "ElaLog.h"
#include "ElaMenu.h"
#include "ElaMenuBar.h"
#include "ElaStatusBar.h"
#include "ElaText.h"
#include "ElaPlainTextEdit.h"
#include "ElaTheme.h"
#include "ElaToolBar.h"
#include "ElaToolButton.h"
#include "ElaMessageBar.h"

// Qt 组件
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QUuid>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>

namespace Prism {
    MainWindow::MainWindow(QWidget *parent)
        : ElaWindow(parent)
          , m_projectManager(std::make_unique<ProjectManager>(this))
    {
        // 初始化窗口
        initWindow();

        // 边缘布局（菜单栏/工具栏/停靠窗口/状态栏）
        initEdgeLayout();

        // 中心内容（页面注册）
        initContent();

        // 拦截默认关闭事件
        _closeDialog = new ElaContentDialog(this);
        _closeDialog->setWindowTitle("退出确认");
        _closeDialog->setLeftButtonText("取消");
        _closeDialog->setMiddleButtonText("最小化");
        _closeDialog->setRightButtonText("退出");
        connect(_closeDialog, &ElaContentDialog::rightButtonClicked, this, &MainWindow::closeWindow);
        connect(_closeDialog, &ElaContentDialog::middleButtonClicked, this, [=]() {
            _closeDialog->close();
            showMinimized();
        });
        this->setIsDefaultClosed(false);
        connect(this, &MainWindow::closeButtonClicked, this, [=]() {
            _closeDialog->exec();
        });

        // 初始化成功提示
        ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功", "Prism 初始化完成", 2000);

        // 加载上次打开的项目
        loadProjectsFromSettings();
    }

    MainWindow::~MainWindow() = default;

    void MainWindow::initWindow()
    {
        setFocusPolicy(Qt::StrongFocus);

        // 窗口基础属性
        setWindowIcon(QIcon(":/Resource/Image/icon.png"));
        resize(1400, 900);
        setWindowTitle("Prism - 仿真配置集成环境");

        // 用户信息卡片（左下角）
        // setUserInfoCardPixmap(QPixmap(":/Resource/Image/avatar.png"));
        setUserInfoCardTitle("Prism Manager");
        setUserInfoCardSubTitle("v1.0.0 - 国腾天创");

        // 可选：设置默认页面
        ElaText *centralStack = new ElaText("欢迎使用 Prism 仿真配置集成环境", this);
        centralStack->setFocusPolicy(Qt::StrongFocus);
        centralStack->setTextPixelSize(28);
        centralStack->setAlignment(Qt::AlignCenter);
        addCentralWidget(centralStack);

        // 自定义 AppBar 右键菜单
        ElaMenu *appBarMenu = new ElaMenu(this);
        appBarMenu->setMenuItemHeight(27);

        // 主题切换
        connect(appBarMenu->addElaIconAction(ElaIconType::MoonStars, "切换主题"),
                &QAction::triggered, this, [=]() {
                    eTheme->setThemeMode(eTheme->getThemeMode() == ElaThemeType::Light
                                             ? ElaThemeType::Dark
                                             : ElaThemeType::Light);
                });

        // 设置菜单
        connect(appBarMenu->addElaIconAction(ElaIconType::GearComplex, "设置"),
                &QAction::triggered, this, [=]() {
                    navigation(_settingKey);
                });

        setCustomMenu(appBarMenu);
    }

    void MainWindow::initEdgeLayout()
    {
        // ========================================
        // 菜单栏 (ElaMenuBar)
        // ========================================
        _menuBar = new ElaMenuBar(this);
        _menuBar->setFixedHeight(30);

        // 创建自定义容器（嵌入到 AppBar 中间区域）
        QWidget *customWidget = new QWidget(this);
        customWidget->setFixedWidth(300);
        QVBoxLayout *customLayout = new QVBoxLayout(customWidget);
        customLayout->setContentsMargins(0, 0, 0, 0);
        customLayout->addWidget(_menuBar);
        customLayout->addStretch();
        this->setCustomWidget(ElaAppBarType::MiddleArea, customWidget);

        // ========================================
        // 文件菜单
        // ========================================
        ElaMenu *fileMenu = _menuBar->addMenu(ElaIconType::Folder, "文件(&F)");
        fileMenu->setMenuItemHeight(27);

        QAction *openProjectAction = fileMenu->addElaIconAction(ElaIconType::FolderOpen, "导入项目\tCtrl+O");
        openProjectAction->setShortcut(QKeySequence::Open);
        connect(openProjectAction, &QAction::triggered, this, &MainWindow::onImportProject);

        fileMenu->addSeparator();

        QAction *saveConfigAction = fileMenu->addElaIconAction(ElaIconType::FloppyDisk, "保存\tCtrl+S");
        saveConfigAction->setShortcut(QKeySequence::Save);
        connect(saveConfigAction, &QAction::triggered, this, &MainWindow::onSaveConfig);

        QAction *closeProjectAction = fileMenu->addElaIconAction(ElaIconType::Xmark, "关闭当前项目\tCtrl+W");
        closeProjectAction->setShortcut(QString("Ctrl+W"));
        connect(closeProjectAction, &QAction::triggered, this, &MainWindow::onCloseCurrentProject);

        QAction *removeProjectAction = fileMenu->addElaIconAction(ElaIconType::TrashCan, "从列表移除");
        connect(removeProjectAction, &QAction::triggered, this, &MainWindow::onRemoveProject);

        fileMenu->addSeparator();

        QAction *exitAction = fileMenu->addElaIconAction(ElaIconType::ArrowRightFromBracket, "退出\tCtrl+Q");
        exitAction->setShortcut(QString("Ctrl+Q"));
        connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);

        // ========================================
        // 项目菜单
        // ========================================
        ElaMenu *projectMenu = _menuBar->addMenu(ElaIconType::Rocket, "项目(&P)");
        projectMenu->setMenuItemHeight(27);

        QAction *projectSettingsAction = projectMenu->addElaIconAction(ElaIconType::GearComplex, "项目设置");
        connect(projectSettingsAction, &QAction::triggered, this, &MainWindow::onProjectSettings);

        QAction *addConfigAction = projectMenu->addElaIconAction(ElaIconType::FilePlus, "添加配置文件");
        connect(addConfigAction, &QAction::triggered, this, &MainWindow::onAddConfigFile);

        projectMenu->addSeparator();

        QAction *refreshAction = projectMenu->addElaIconAction(ElaIconType::ArrowRotateRight, "刷新项目\tF5");
        refreshAction->setShortcut(Qt::Key_F5);
        connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshProject);

        QAction *openExplorerAction = projectMenu->addElaIconAction(ElaIconType::FolderOpen, "在资源管理器中打开");
        connect(openExplorerAction, &QAction::triggered, this, &MainWindow::onOpenInExplorer);

        // ========================================
        // 帮助菜单
        // ========================================
        ElaMenu *helpMenu = _menuBar->addMenu(ElaIconType::CircleInfo, "帮助(&H)");
        helpMenu->setMenuItemHeight(27);
        helpMenu->addElaIconAction(ElaIconType::BookOpen, "文档");
        helpMenu->addElaIconAction(ElaIconType::User, "关于");

        // ========================================
        // 工具栏 (ElaToolBar)
        // ========================================
        ElaToolBar *toolBar = new ElaToolBar("主工具栏", this);
        toolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
        toolBar->setToolBarSpacing(3);
        toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolBar->setIconSize(QSize(25, 25));

        ElaToolButton *openButton = new ElaToolButton(this);
        openButton->setElaIcon(ElaIconType::FolderOpen);
        openButton->setToolTip("导入项目 (Ctrl+O)");
        connect(openButton, &ElaToolButton::clicked, this, &MainWindow::onImportProject);
        toolBar->addWidget(openButton);

        toolBar->addSeparator();

        ElaToolButton *runButton = new ElaToolButton(this);
        runButton->setElaIcon(ElaIconType::Play);
        runButton->setToolTip("运行 (F5)");
        runButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolBar->addWidget(runButton);

        ElaToolButton *stopButton = new ElaToolButton(this);
        stopButton->setElaIcon(ElaIconType::Stop);
        stopButton->setToolTip("停止 (Shift+F5)");
        toolBar->addWidget(stopButton);

        this->addToolBar(Qt::TopToolBarArea, toolBar);

        // ========================================
        // 停靠窗口 (ElaDockWidget) - 日志输出
        // ========================================
        ElaDockWidget *logDockWidget = new ElaDockWidget("日志输出", this);

        // 创建日志文本编辑器
        ElaPlainTextEdit *logTextEdit = new ElaPlainTextEdit(this);
        logTextEdit->setReadOnly(true);
        logTextEdit->setStyleSheet(
            "ElaPlainTextEdit { "
            "background-color: #1E1E1E; "
            "color: #D4D4D4; "
            "font-family: 'Consolas', 'Courier New', monospace; "
            "font-size: 11pt; "
            "}"
        );
        logTextEdit->appendPlainText("[INFO] Prism 管理器已初始化");
        logTextEdit->appendPlainText("[INFO] 等待用户操作...");

        logDockWidget->setWidget(logTextEdit);
        this->addDockWidget(Qt::BottomDockWidgetArea, logDockWidget);
        resizeDocks({logDockWidget}, {250}, Qt::Vertical);

        // ========================================
        // 状态栏 (ElaStatusBar)
        // ========================================
        _statusBar = new ElaStatusBar(this);
        ElaText *statusText = new ElaText("就绪 ", this);
        statusText->setTextPixelSize(15);
        _statusBar->addWidget(statusText);
        this->setStatusBar(_statusBar);
    }

    void MainWindow::initContent()
    {
        // ========================================
        // 创建通用页面实例
        // ========================================
        _homePage = new HomePage(this);
        _settingPage = new BasePage(this);

        // ========================================
        // 注册顶级导航节点
        // ========================================
        addPageNode("首页", _homePage, ElaIconType::House);

        // ========================================
        // 项目管理展开器节点（顶级）
        // ========================================
        // 预留空间：后续通过 addProject() 动态添加项目子节点
        // 每个项目将作为一个展开器节点，包含"配置文件"和"进程监控"页面
        addExpanderNode("项目管理", _projectManagementKey, ElaIconType::FolderTree);

        // TODO: 在项目管理下动态添加项目
        // 示例结构：
        // └── 项目管理 (展开器)
        //     ├── [项目A] (展开器)
        //     │   ├── 配置文件
        //     │   └── 进程监控
        //     └── [项目B] (展开器)
        //         ├── 配置文件
        //         └── 进程监控

        // ========================================
        // 底部固定节点
        // ========================================
        addFooterNode("设置", _settingPage, _settingKey, 0, ElaIconType::GearComplex);

        // ========================================
        // 连接导航信号
        // ========================================
        connect(this, &ElaWindow::navigationNodeClicked, this,
                [=](ElaNavigationType::NavigationNodeType nodeType, QString nodeKey) {
                    // 处理特殊节点点击
                    if (nodeKey == _settingKey)
                    {
                        qDebug() << "设置页面被点击";
                        return;
                    }

                    // 根据页面Key更新当前项目
                    updateCurrentProjectFromPageKey(nodeKey);
                });

        // 用户信息卡片点击 -> 跳转到首页
        connect(this, &MainWindow::userInfoCardClicked, this, [=]() {
            this->navigation(_homePage->property("ElaPageKey").toString());
        });
    }

    // ========================================
    // 项目管理公共接口实现
    // ========================================
    QString MainWindow::addProject(const QString &projectPath, bool silent)
    {
        // 检查项目是否已存在（按路径）
        for (const auto& project : _openedProjects) {
            if (project.rootPath == projectPath) {
                if (!silent) {
                    ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                          QString("项目已打开: %1").arg(project.name), 2000);
                }
                return project.id;
            }
        }

        // 验证路径有效性
        QDir projectDir(projectPath);
        if (!projectDir.exists()) {
            if (!silent) {
                ElaMessageBar::error(ElaMessageBarType::BottomRight, "错误",
                                    "项目目录不存在", 2000);
            }
            return QString();
        }

        // 生成唯一 ID (UUID)
        QString projectId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        qDebug() << "添加项目:" << projectPath << "ID:" << projectId;

        // 创建项目信息
        ProjectInfo info;
        info.id = projectId;
        info.name = projectPath.section('/', -1).section('\\', -1); // 兼容 Windows 路径
        info.rootPath = projectPath;
        info.lastOpened = QDateTime::currentDateTime();
        info.isActive = false;

        // 扫描配置文件
        QStringList nameFilters = {"*.ini", "*.json", "*.yaml", "*.yml", "*.conf"};
        QFileInfoList fileInfoList = projectDir.entryInfoList(nameFilters, QDir::Files);
        for (const QFileInfo &fileInfo: fileInfoList) {
            info.configFiles.append(fileInfo.absoluteFilePath());
        }

        _openedProjects[projectId] = info;

        // 创建项目专属页面
        ConfigPage *configPage = new ConfigPage(this);
        configPage->setProjectName(info.name);
        configPage->loadConfigFiles(info.configFiles);

        ProcessPage *processPage = new ProcessPage(this);
        processPage->setProjectName(info.name);

        // 注册到导航树
        QString projectExpanderKey;
        addExpanderNode(info.name, projectExpanderKey, _projectManagementKey, ElaIconType::Folder);
        addPageNode("配置文件", configPage, projectExpanderKey, 0, ElaIconType::FileCode);
        addPageNode("进程监控", processPage, projectExpanderKey, ElaIconType::Terminal);

        // 获取页面的 Key（用于反向查找）
        QString configPageKey = configPage->property("ElaPageKey").toString();
        QString processPageKey = processPage->property("ElaPageKey").toString();

        // 保存引用（使用 projectId 作为 key）
        _projectConfigPages[projectId] = configPage;
        _projectProcessPages[projectId] = processPage;
        _projectExpanderKeys[projectId] = projectExpanderKey;

        // 建立页面Key到项目ID的反向映射
        _pageKeyToProjectId[configPageKey] = projectId;
        _pageKeyToProjectId[processPageKey] = projectId;
        _pageKeyToProjectId[projectExpanderKey] = projectId;

        // 保存项目列表
        saveProjectsToSettings();

        if (!silent) {
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                                   QString("项目已添加: %1").arg(info.name), 2000);
        }

        return projectId;
    }

    void MainWindow::removeProject(const QString &projectId)
    {
        if (!_openedProjects.contains(projectId)) {
            qWarning() << "项目不存在:" << projectId;
            return;
        }

        QString projectName = _openedProjects[projectId].name;
        QString expanderKey = _projectExpanderKeys.value(projectId);

        // 清理页面Key到项目ID的映射
        QStringList keysToRemove;
        for (auto it = _pageKeyToProjectId.begin(); it != _pageKeyToProjectId.end(); ++it) {
            if (it.value() == projectId) {
                keysToRemove.append(it.key());
            }
        }
        for (const QString& key : keysToRemove) {
            _pageKeyToProjectId.remove(key);
        }

        // 移除导航节点（这会同时移除展开器下的所有子节点）
        if (!expanderKey.isEmpty()) {
            removeNavigationNode(expanderKey);
        }

        // 释放配置页面
        if (_projectConfigPages.contains(projectId)) {
            _projectConfigPages[projectId]->deleteLater();
            _projectConfigPages.remove(projectId);
        }

        // 释放进程页面
        if (_projectProcessPages.contains(projectId)) {
            _projectProcessPages[projectId]->deleteLater();
            _projectProcessPages.remove(projectId);
        }

        // 清理其他映射
        _openedProjects.remove(projectId);
        _projectExpanderKeys.remove(projectId);

        // 如果移除的是当前项目，清空当前项目ID
        if (_currentProjectId == projectId) {
            _currentProjectId.clear();
        }

        // 保存项目列表
        saveProjectsToSettings();

        ElaMessageBar::information(ElaMessageBarType::BottomRight, "信息",
                                   QString("项目已移除: %1").arg(projectName), 2000);
    }

    void MainWindow::deactivateProject(const QString &projectId)
    {
        if (!_openedProjects.contains(projectId)) {
            qWarning() << "项目不存在:" << projectId;
            return;
        }

        QString projectName = _openedProjects[projectId].name;

        // 取消激活状态
        _openedProjects[projectId].isActive = false;

        // 清空当前项目ID
        if (_currentProjectId == projectId) {
            _currentProjectId.clear();
        }

        // 导航到首页
        navigation(_homePage->property("ElaPageKey").toString());

        ElaMessageBar::information(ElaMessageBarType::BottomRight, "信息",
                                   QString("已关闭项目: %1").arg(projectName), 1500);
    }

    void MainWindow::setActiveProject(const QString &projectId)
    {
        // TODO: 实现切换激活项目
        // 1. 验证项目 ID 存在
        // 2. 取消之前的激活状态
        // 3. 设置新的激活状态
        // 4. 更新状态栏显示

        if (!_openedProjects.contains(projectId))
        {
            qWarning() << "项目不存在:" << projectId;
            return;
        }

        // 取消之前的激活状态
        for (auto &project: _openedProjects)
        {
            project.isActive = false;
        }

        // 激活新项目
        _openedProjects[projectId].isActive = true;
        _currentProjectId = projectId;

        qDebug() << "激活项目:" << _openedProjects[projectId].name;
    }

    // ========================================
    // 菜单栏槽函数实现
    // ========================================
    void MainWindow::onImportProject()
    {
        QString projectPath = QFileDialog::getExistingDirectory(
            this,
            "选择项目目录",
            QDir::homePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

        if (!projectPath.isEmpty())
        {
            addProject(projectPath);
        }
    }

    void MainWindow::onSaveConfig()
    {
        if (_currentProjectId.isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "未选择项目", 1500);
            return;
        }

        // 获取当前项目的 ConfigPage 并保存
        if (_projectConfigPages.contains(_currentProjectId))
        {
            ConfigPage* configPage = qobject_cast<ConfigPage*>(_projectConfigPages[_currentProjectId]);
            if (configPage)
            {
                configPage->saveCurrentConfig();
            }
        }
    }

    void MainWindow::onCloseCurrentProject()
    {
        if (_currentProjectId.isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "未选择项目", 1500);
            return;
        }

        deactivateProject(_currentProjectId);
    }

    void MainWindow::onRemoveProject()
    {
        if (_currentProjectId.isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "未选择项目", 1500);
            return;
        }

        QString projectName = _openedProjects[_currentProjectId].name;
        QString projectId = _currentProjectId;

        // 创建确认对话框
        ElaContentDialog *confirmDialog = new ElaContentDialog(this);
        confirmDialog->setWindowTitle("移除项目");
        confirmDialog->setLeftButtonText("取消");
        confirmDialog->setMiddleButtonText("仅关闭");
        confirmDialog->setRightButtonText("从列表移除");

        // 创建对话框内容
        QWidget *contentWidget = new QWidget(confirmDialog);
        QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(20, 20, 20, 20);

        ElaText *messageText = new ElaText(contentWidget);
        messageText->setText(QString("请选择对项目 \"%1\" 的操作：").arg(projectName));
        messageText->setTextPixelSize(15);

        ElaText *hintText = new ElaText(contentWidget);
        hintText->setText("• 仅关闭：取消激活，项目仍保留在列表中\n• 从列表移除：彻底移除（本地文件不受影响）");
        hintText->setTextPixelSize(12);

        contentLayout->addWidget(messageText);
        contentLayout->addSpacing(10);
        contentLayout->addWidget(hintText);
        contentLayout->addStretch();

        confirmDialog->setCentralWidget(contentWidget);

        // 仅关闭（取消激活）
        connect(confirmDialog, &ElaContentDialog::middleButtonClicked, this, [=]() {
            confirmDialog->close();
            deactivateProject(projectId);
        });

        // 从列表移除
        connect(confirmDialog, &ElaContentDialog::rightButtonClicked, this, [=]() {
            confirmDialog->close();
            removeProject(projectId);
        });

        confirmDialog->exec();
    }

    void MainWindow::onExit()
    {
        // 触发关闭确认对话框
        _closeDialog->exec();
    }

    void MainWindow::onProjectSettings()
    {
        if (_currentProjectId.isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "未选择项目", 1500);
            return;
        }

        // TODO: 打开项目设置对话框
        ElaMessageBar::information(ElaMessageBarType::BottomRight, "提示",
                                   "项目设置功能即将推出", 2000);
    }

    void MainWindow::onAddConfigFile()
    {
        if (_currentProjectId.isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "未选择项目", 1500);
            return;
        }

        // TODO: 打开添加配置文件对话框
        ElaMessageBar::information(ElaMessageBarType::BottomRight, "提示",
                                   "添加配置文件功能即将推出", 2000);
    }

    void MainWindow::onRefreshProject()
    {
        if (_currentProjectId.isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "未选择项目", 1500);
            return;
        }

        // TODO: 刷新项目文件列表
        ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                               "项目已刷新", 1500);
    }

    void MainWindow::onOpenInExplorer()
    {
        if (_currentProjectId.isEmpty())
        {
            ElaMessageBar::warning(ElaMessageBarType::BottomRight, "警告",
                                   "未选择项目", 1500);
            return;
        }

        QString projectPath = _openedProjects[_currentProjectId].rootPath;
        QDesktopServices::openUrl(QUrl::fromLocalFile(projectPath));
    }

    // ========================================
    // 持久化和辅助方法实现
    // ========================================
    void MainWindow::saveProjectsToSettings()
    {
        QSettings settings("GTTC", "Prism");
        settings.beginGroup("Projects");

        // 清除旧数据
        settings.remove("");

        // 保存项目列表
        QStringList projectPaths;
        for (const auto& project : _openedProjects) {
            projectPaths.append(project.rootPath);
        }
        settings.setValue("paths", projectPaths);

        settings.endGroup();
        settings.sync();

        qDebug() << "已保存项目列表:" << projectPaths.size() << "个项目";
    }

    void MainWindow::loadProjectsFromSettings()
    {
        QSettings settings("GTTC", "Prism");
        settings.beginGroup("Projects");

        QStringList projectPaths = settings.value("paths").toStringList();
        settings.endGroup();

        qDebug() << "加载项目列表:" << projectPaths.size() << "个项目";

        // 重新打开每个项目（静默模式）
        int loadedCount = 0;
        for (const QString& path : projectPaths) {
            QDir dir(path);
            if (dir.exists()) {
                if (!addProject(path, true).isEmpty()) {
                    loadedCount++;
                }
            } else {
                qWarning() << "项目目录不存在，跳过:" << path;
            }
        }

        // 显示汇总消息
        if (loadedCount > 0) {
            ElaMessageBar::success(ElaMessageBarType::BottomRight, "成功",
                                   QString("已恢复 %1 个项目").arg(loadedCount), 2000);
        }
    }

    void MainWindow::updateCurrentProjectFromPageKey(const QString& pageKey)
    {
        // 通过页面Key查找项目ID
        if (_pageKeyToProjectId.contains(pageKey)) {
            QString projectId = _pageKeyToProjectId[pageKey];
            if (projectId != _currentProjectId) {
                setActiveProject(projectId);
                qDebug() << "切换当前项目:" << _openedProjects[projectId].name;
            }
        }
    }
} // namespace Prism
