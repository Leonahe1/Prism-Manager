#include "HomePage.h"
#include "MainWindow.h"

#include <QDebug>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QShowEvent>
#include <QVBoxLayout>
#include <algorithm>

#include "ElaIconButton.h"
#include "ElaMenu.h"
#include "ElaMessageBar.h"
#include "ElaPushButton.h"
#include "ElaScrollArea.h"
#include "ElaText.h"
#include "ElaTheme.h"

HomePage::HomePage(QWidget* parent)
    : BasePage(parent)
{
    setWindowTitle("首页");
    setTitleVisible(false);
    setContentsMargins(2, 2, 0, 0);

    initUI();
    // setupConnections() 延迟到 showEvent 中调用，因为此时 window() 还未返回有效的 MainWindow

    qDebug() << "主页初始化完成";
}

HomePage::~HomePage()
{
}

void HomePage::initUI()
{
    // 主容器
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("首页");
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // ========== 标题区域 ==========
    ElaText* desText = new ElaText("仿真配置集成环境", this);
    desText->setTextPixelSize(18);
    mainLayout->addWidget(desText);

    ElaText* titleText = new ElaText("Prism Manager", this);
    titleText->setTextPixelSize(35);
    mainLayout->addWidget(titleText);

    mainLayout->addSpacing(10);

    // ========== 统计卡片区域 ==========
    _statsContainer = new QWidget(centralWidget);
    QHBoxLayout* statsLayout = new QHBoxLayout(_statsContainer);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    statsLayout->setSpacing(15);

    _totalProjectsCard = createStatCard("总项目数", "0", ElaIconType::Folder);
    _totalConfigsCard = createStatCard("配置文件", "0", ElaIconType::FileCode);
    _runningProcessCard = createStatCard("运行进程", "0", ElaIconType::Terminal);

    // 保存数值文本指针（用于更新）
    _totalProjectsValue = _totalProjectsCard->findChild<ElaText*>("valueText");
    _totalConfigsValue = _totalConfigsCard->findChild<ElaText*>("valueText");
    _runningProcessValue = _runningProcessCard->findChild<ElaText*>("valueText");

    statsLayout->addWidget(_totalProjectsCard);
    statsLayout->addWidget(_totalConfigsCard);
    statsLayout->addWidget(_runningProcessCard);

    mainLayout->addWidget(_statsContainer);

    // ========== 内容区域（左右分栏）==========
    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(15);

    // 左侧：最近项目列表面板
    _recentProjectsPanel = new QWidget(centralWidget);
    _recentProjectsPanel->setObjectName("RecentProjectsPanel");
    QVBoxLayout* leftLayout = new QVBoxLayout(_recentProjectsPanel);
    leftLayout->setContentsMargins(16, 16, 16, 16);
    leftLayout->setSpacing(10);

    ElaText* recentTitle = new ElaText("最近打开的项目", _recentProjectsPanel);
    recentTitle->setTextPixelSize(16);
    leftLayout->addWidget(recentTitle);

    // 滚动区域
    _recentProjectsScrollArea = new ElaScrollArea(_recentProjectsPanel);
    _recentProjectsScrollArea->setWidgetResizable(true);
    _recentProjectsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _recentProjectsScrollArea->setMinimumHeight(250);

    _recentProjectsContainer = new QWidget(_recentProjectsScrollArea);
    _recentProjectsContainer->setStyleSheet("background: transparent;");
    _recentProjectsLayout = new QVBoxLayout(_recentProjectsContainer);
    _recentProjectsLayout->setContentsMargins(5, 5, 5, 5);
    _recentProjectsLayout->setSpacing(10);
    _recentProjectsLayout->addStretch();

    _recentProjectsScrollArea->setWidget(_recentProjectsContainer);
    leftLayout->addWidget(_recentProjectsScrollArea);

    // 空状态提示
    _emptyProjectsHint = new ElaText("暂无项目，请先导入或创建项目", _recentProjectsPanel);
    _emptyProjectsHint->setTextPixelSize(14);
    _emptyProjectsHint->setAlignment(Qt::AlignCenter);
    _emptyProjectsHint->setMinimumHeight(100);
    _emptyProjectsHint->setVisible(true);  // 默认显示
    leftLayout->addWidget(_emptyProjectsHint);

    // 默认隐藏滚动区域（无项目时）
    _recentProjectsScrollArea->setVisible(false);

    contentLayout->addWidget(_recentProjectsPanel, 7);  // 70% 宽度

    // 右侧：快捷操作面板
    _quickActionsContainer = new QWidget(centralWidget);
    _quickActionsContainer->setObjectName("QuickActionsPanel");
    _quickActionsContainer->setMinimumWidth(200);

    QVBoxLayout* quickLayout = new QVBoxLayout(_quickActionsContainer);
    quickLayout->setContentsMargins(16, 16, 16, 16);
    quickLayout->setSpacing(12);

    ElaText* quickTitle = new ElaText("快捷操作", _quickActionsContainer);
    quickTitle->setTextPixelSize(16);
    quickLayout->addWidget(quickTitle);

    _openConfigButton = new ElaPushButton("打开配置", _quickActionsContainer);
    _openConfigButton->setFixedHeight(45);
    connect(_openConfigButton, &ElaPushButton::clicked,
            this, &HomePage::onOpenConfigClicked);
    quickLayout->addWidget(_openConfigButton);

    quickLayout->addStretch();

    contentLayout->addWidget(_quickActionsContainer, 3);  // 30% 宽度

    mainLayout->addLayout(contentLayout, 1);

    // ========== 右键菜单 ==========
    _homeMenu = new ElaMenu(this);
    _homeMenu->addElaIconAction(ElaIconType::ArrowRotateRight, "刷新");
    _homeMenu->addSeparator();

    ElaMenu* viewMenu = _homeMenu->addMenu(ElaIconType::Eye, "视图");
    viewMenu->addAction("配置管理");
    viewMenu->addAction("进程监控");
    viewMenu->addAction("项目管理");

    _homeMenu->addElaIconAction(ElaIconType::GearComplex, "设置");

    // 添加到页面
    addCentralWidget(centralWidget, true, false, 0);

    // 初始化主题
    updateTheme();
}

QWidget* HomePage::createStatCard(const QString& title, const QString& value, int iconType)
{
    QWidget* card = new QWidget();
    card->setObjectName("StatCard");
    card->setFixedHeight(80);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 横向布局：图标 | 数值和标签
    QHBoxLayout* mainLayout = new QHBoxLayout(card);
    mainLayout->setContentsMargins(20, 15, 20, 15);
    mainLayout->setSpacing(15);

    // 左侧：图标
    ElaIconButton* iconBtn = new ElaIconButton(static_cast<ElaIconType::IconName>(iconType), card);
    iconBtn->setFixedSize(40, 40);
    iconBtn->setEnabled(false);  // 仅作为装饰
    mainLayout->addWidget(iconBtn);

    // 右侧：数值和标签（垂直排列）
    QVBoxLayout* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);

    // 数值
    ElaText* valueText = new ElaText(value, card);
    valueText->setObjectName("valueText");
    valueText->setTextPixelSize(24);
    textLayout->addWidget(valueText);

    // 标签
    ElaText* titleText = new ElaText(title, card);
    titleText->setTextPixelSize(12);
    textLayout->addWidget(titleText);

    mainLayout->addLayout(textLayout);
    mainLayout->addStretch();

    return card;
}

QWidget* HomePage::createProjectCard(const QString& id, const QString& name,
                                     const QString& path, const QDateTime& lastOpened,
                                     int configCount)
{
    QWidget* card = new QWidget();
    card->setObjectName("ProjectCard");
    card->setProperty("projectId", id);
    card->setCursor(Qt::PointingHandCursor);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    card->setFixedHeight(100);
    card->installEventFilter(this);

    // 主布局：左侧图标 | 右侧内容
    QHBoxLayout* mainLayout = new QHBoxLayout(card);
    mainLayout->setContentsMargins(16, 12, 16, 12);
    mainLayout->setSpacing(12);

    // 左侧：文件夹图标
    ElaIconButton* folderIcon = new ElaIconButton(ElaIconType::Folder, card);
    folderIcon->setFixedSize(36, 36);
    folderIcon->setEnabled(false);
    mainLayout->addWidget(folderIcon, 0, Qt::AlignTop);

    // 右侧：项目信息（垂直布局）
    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);
    infoLayout->setContentsMargins(0, 0, 0, 0);

    // 项目名称
    ElaText* nameText = new ElaText(name, card);
    nameText->setTextPixelSize(15);
    infoLayout->addWidget(nameText);

    // 路径（使用省略号处理过长路径）
    ElaText* pathText = new ElaText(path, card);
    pathText->setTextPixelSize(11);
    pathText->setWordWrap(false);
    infoLayout->addWidget(pathText);

    // 底部信息行：时间 + 配置文件数
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(0);

    QString timeStr = lastOpened.toString("yyyy-MM-dd HH:mm");
    ElaText* timeText = new ElaText(QString("最后打开: %1").arg(timeStr), card);
    timeText->setTextPixelSize(12);
    bottomLayout->addWidget(timeText);

    bottomLayout->addStretch();

    ElaText* configText = new ElaText(QString("%1 个配置文件").arg(configCount), card);
    configText->setTextPixelSize(10);
    bottomLayout->addWidget(configText);

    infoLayout->addLayout(bottomLayout);
    infoLayout->addStretch();

    mainLayout->addLayout(infoLayout, 1);

    return card;
}

void HomePage::setupConnections()
{
    Prism::MainWindow* mainWin = qobject_cast<Prism::MainWindow*>(window());
    if (mainWin) {
        connect(mainWin, &Prism::MainWindow::projectDataChanged,
                this, &HomePage::refreshData);
    }

    connect(eTheme, &ElaTheme::themeModeChanged,
            this, &HomePage::onThemeChanged);
}

void HomePage::refreshData()
{
    updateStatistics();
    updateRecentProjects();
}

void HomePage::updateStatistics()
{
    Prism::MainWindow* mainWin = qobject_cast<Prism::MainWindow*>(window());
    if (!mainWin) return;

    if (_totalProjectsValue) {
        _totalProjectsValue->setText(QString::number(mainWin->getOpenedProjectsCount()));
    }

    if (_totalConfigsValue) {
        _totalConfigsValue->setText(QString::number(mainWin->getTotalConfigFilesCount()));
    }

    if (_runningProcessValue) {
        _runningProcessValue->setText(QString::number(mainWin->getRunningProcessCount()));
    }
}

void HomePage::updateRecentProjects()
{
    // 清除旧卡片
    for (QWidget* card : _projectCards) {
        _recentProjectsLayout->removeWidget(card);
        card->deleteLater();
    }
    _projectCards.clear();

    Prism::MainWindow* mainWin = qobject_cast<Prism::MainWindow*>(window());
    if (!mainWin) return;

    QList<Prism::ProjectInfo> projects = mainWin->getAllProjects();

    // 按最后打开时间排序
    std::sort(projects.begin(), projects.end(),
        [](const Prism::ProjectInfo& a, const Prism::ProjectInfo& b) {
            return a.lastOpened > b.lastOpened;
        });

    // 取前5个
    int count = qMin(5, projects.size());

    if (count == 0) {
        _emptyProjectsHint->setVisible(true);
        _recentProjectsScrollArea->setVisible(false);
        return;
    }

    _emptyProjectsHint->setVisible(false);
    _recentProjectsScrollArea->setVisible(true);

    // 创建项目卡片
    for (int i = 0; i < count; ++i) {
        const Prism::ProjectInfo& proj = projects[i];

        QWidget* card = createProjectCard(
            proj.id,
            proj.name,
            proj.rootPath,
            proj.lastOpened,
            proj.configFiles.size()
        );

        _recentProjectsLayout->insertWidget(_recentProjectsLayout->count() - 1, card);
        _projectCards.append(card);
    }

    updateTheme();
}

QString HomePage::getCardStyleSheet() const
{
    bool isDark = eTheme->getThemeMode() == ElaThemeType::Dark;

    QString statCardStyle = QString(
        "QWidget#StatCard {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 10px;"
        "}"
    ).arg(isDark ? "#2D2D2D" : "#FFFFFF",
          isDark ? "#404040" : "#E0E0E0");

    QString projectCardStyle = QString(
        "QWidget#ProjectCard {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 8px;"
        "}"
        "QWidget#ProjectCard:hover {"
        "    background-color: %3;"
        "    border: 1px solid %4;"
        "}"
    ).arg(isDark ? "#2D2D2D" : "#FFFFFF",
          isDark ? "#404040" : "#E0E0E0",
          isDark ? "#353535" : "#F5F5F5",
          isDark ? "#505050" : "#D0D0D0");

    QString recentProjectsPanelStyle = QString(
        "QWidget#RecentProjectsPanel {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 10px;"
        "}"
    ).arg(isDark ? "#2D2D2D" : "#FFFFFF",
          isDark ? "#404040" : "#E0E0E0");

    QString quickActionsStyle = QString(
        "QWidget#QuickActionsPanel {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 10px;"
        "}"
    ).arg(isDark ? "#2D2D2D" : "#FFFFFF",
          isDark ? "#404040" : "#E0E0E0");

    return statCardStyle + "\n" + projectCardStyle + "\n" + recentProjectsPanelStyle + "\n" + quickActionsStyle;
}

void HomePage::updateTheme()
{
    QString styleSheet = getCardStyleSheet();

    if (_totalProjectsCard) _totalProjectsCard->setStyleSheet(styleSheet);
    if (_totalConfigsCard) _totalConfigsCard->setStyleSheet(styleSheet);
    if (_runningProcessCard) _runningProcessCard->setStyleSheet(styleSheet);

    for (QWidget* card : _projectCards) {
        card->setStyleSheet(styleSheet);
    }

    if (_recentProjectsPanel) {
        _recentProjectsPanel->setStyleSheet(styleSheet);
    }

    if (_quickActionsContainer) {
        _quickActionsContainer->setStyleSheet(styleSheet);
    }
}

void HomePage::onThemeChanged()
{
    updateTheme();
}

bool HomePage::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QWidget* card = qobject_cast<QWidget*>(obj);
        if (card && card->objectName() == "ProjectCard") {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                QString projectId = card->property("projectId").toString();
                onProjectCardClicked(projectId);
                return true;
            }
        }
    }
    return BasePage::eventFilter(obj, event);
}

void HomePage::showEvent(QShowEvent* event)
{
    BasePage::showEvent(event);

    if (!_isInitialized) {
        _isInitialized = true;
        setupConnections();
        refreshData();
        qDebug() << "首页首次显示，已建立信号连接并刷新数据";
    }
}

void HomePage::onProjectCardClicked(const QString& projectId)
{
    Prism::MainWindow* mainWin = qobject_cast<Prism::MainWindow*>(window());
    if (!mainWin) return;

    // 激活项目
    mainWin->setActiveProject(projectId);

    // 导航到项目的配置页面
    QString configPageKey = mainWin->getProjectConfigPageKey(projectId);
    if (!configPageKey.isEmpty()) {
        mainWin->navigation(configPageKey);
    }
}

void HomePage::onOpenConfigClicked()
{
    Prism::MainWindow* mainWin = qobject_cast<Prism::MainWindow*>(window());
    if (!mainWin) return;

    // 获取当前项目列表
    QList<Prism::ProjectInfo> projects = mainWin->getAllProjects();

    // 找到当前激活的项目
    Prism::ProjectInfo* currentProject = nullptr;
    for (auto& proj : projects) {
        if (proj.isActive) {
            currentProject = &proj;
            break;
        }
    }

    if (currentProject) {
        QString configPageKey = mainWin->getProjectConfigPageKey(currentProject->id);
        if (!configPageKey.isEmpty()) {
            mainWin->navigation(configPageKey);
            return;
        }
    }

    // 否则提示用户先打开项目
    ElaMessageBar::information(ElaMessageBarType::Top, "提示", "请先打开或创建一个项目", 2000, mainWin);
}

void HomePage::mouseReleaseEvent(QMouseEvent* event)
{
    switch (event->button())
    {
    case Qt::RightButton:
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        _homeMenu->popup(event->globalPosition().toPoint());
#else
        _homeMenu->popup(event->globalPos());
#endif
        break;
    }
    default:
    {
        break;
    }
    }
    ElaScrollPage::mouseReleaseEvent(event);
}
