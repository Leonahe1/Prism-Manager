#include "HomePage.h"

#include <QDebug>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>

#include "ElaMenu.h"
#include "ElaMessageBar.h"
#include "ElaNavigationRouter.h"
#include "ElaScrollArea.h"
#include "ElaText.h"

HomePage::HomePage(QWidget* parent)
    : BasePage(parent)
{
    // 窗口标题
    setWindowTitle("首页");
    setTitleVisible(false);
    setContentsMargins(2, 2, 0, 0);
    // ========================================
    // 标题区域
    // ========================================
    ElaText* desText = new ElaText("仿真配置集成环境", this);
    desText->setTextPixelSize(18);

    ElaText* titleText = new ElaText("Prism Manager", this);
    titleText->setTextPixelSize(35);

    QVBoxLayout* titleLayout = new QVBoxLayout();
    titleLayout->setContentsMargins(30, 30, 0, 0);
    titleLayout->addWidget(desText);
    titleLayout->addWidget(titleText);
    titleLayout->addSpacing(20);

    // 欢迎文本
    ElaText* welcomeText = new ElaText("欢迎使用 Prism 管理器！", this);
    welcomeText->setTextPixelSize(16);
    welcomeText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    titleLayout->addWidget(welcomeText);

    titleLayout->addSpacing(20);

    // 快速开始提示
    ElaText* quickStartText = new ElaText("📌 快速开始：通过「文件」菜单打开或创建项目", this);
    quickStartText->setTextPixelSize(14);
    quickStartText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    titleLayout->addWidget(quickStartText);

    // ========================================
    // 预留区域：后续可添加最近项目列表、统计信息等
    // ========================================
    // TODO: 添加最近打开的项目列表
    // TODO: 添加项目统计信息卡片
    // TODO: 添加快捷操作面板

    // ========================================
    // 右键菜单
    // ========================================
    _homeMenu = new ElaMenu(this);
    _homeMenu->addElaIconAction(ElaIconType::ArrowRotateRight, "刷新");
    _homeMenu->addSeparator();

    ElaMenu* viewMenu = _homeMenu->addMenu(ElaIconType::Eye, "视图");
    viewMenu->addAction("配置管理");
    viewMenu->addAction("进程监控");
    viewMenu->addAction("项目管理");

    _homeMenu->addElaIconAction(ElaIconType::GearComplex, "设置");

    // ========================================
    // 组装中央布局
    // ========================================
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setWindowTitle("首页");
    QVBoxLayout* centerVLayout = new QVBoxLayout(centralWidget);
    centerVLayout->setSpacing(0);
    centerVLayout->setContentsMargins(0, 0, 0, 0);
    centerVLayout->addLayout(titleLayout);
    centerVLayout->addStretch();
    addCentralWidget(centralWidget);

    qDebug() << "主页初始化完成";
}

HomePage::~HomePage()
{
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
