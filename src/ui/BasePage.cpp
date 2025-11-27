#include "BasePage.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "ElaMenu.h"
#include "ElaText.h"
#include "ElaTheme.h"
#include "ElaToolButton.h"
#include "ElaWindow.h"

BasePage::BasePage(QWidget* parent)
    : ElaScrollPage(parent)
{
    // 监听主题变更，自动更新界面
    connect(eTheme, &ElaTheme::themeModeChanged, this, [=]() {
        if (!parent)
        {
            update();
        }
    });

    setContentsMargins(20, 5, 0, 0);
}

BasePage::~BasePage()
{
}

void BasePage::createCustomWidget(QString desText)
{
    // 顶部元素
    QWidget* customWidget = new QWidget(this);

    // 项目链接
    ElaText* subTitleText = new ElaText(this);
    subTitleText->setText("https://github.com/yourproject/prism-manager");
    subTitleText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    subTitleText->setTextPixelSize(11);

    // 文档按钮
    ElaToolButton* documentationButton = new ElaToolButton(this);
    documentationButton->setFixedHeight(35);
    documentationButton->setIsTransparent(false);
    documentationButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    documentationButton->setText("文档");
    documentationButton->setElaIcon(ElaIconType::FileDoc);
    ElaMenu* documentationMenu = new ElaMenu(this);
    documentationMenu->addElaIconAction(ElaIconType::BookOpen, "用户手册");
    documentationMenu->addElaIconAction(ElaIconType::Code, "开发文档");
    documentationButton->setMenu(documentationMenu);

    // 源码按钮
    ElaToolButton* sourceButton = new ElaToolButton(this);
    sourceButton->setFixedHeight(35);
    sourceButton->setIsTransparent(false);
    sourceButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sourceButton->setText("源码");
    sourceButton->setElaIcon(ElaIconType::Code);
    ElaMenu* sourceMenu = new ElaMenu(this);
    sourceMenu->addElaIconAction(ElaIconType::CodeBranch, "查看源码");
    sourceMenu->addElaIconAction(ElaIconType::Bug, "报告问题");
    sourceButton->setMenu(sourceMenu);

    // 主题切换按钮
    ElaToolButton* themeButton = new ElaToolButton(this);
    themeButton->setFixedSize(35, 35);
    themeButton->setIsTransparent(false);
    themeButton->setElaIcon(ElaIconType::MoonStars);
    connect(themeButton, &ElaToolButton::clicked, this, [=]() {
        eTheme->setThemeMode(eTheme->getThemeMode() == ElaThemeType::Light
                             ? ElaThemeType::Dark : ElaThemeType::Light);
    });

    // 返回按钮
    ElaToolButton* backtrackButton = new ElaToolButton(this);
    backtrackButton->setFixedSize(35, 35);
    backtrackButton->setIsTransparent(false);
    backtrackButton->setElaIcon(ElaIconType::ArrowLeft);
    connect(backtrackButton, &ElaToolButton::clicked, this, [=]() {
        ElaWindow* window = dynamic_cast<ElaWindow*>(this->window());
        if (window)
        {
            window->backtrackNavigationNode(property("ElaPageKey").toString());
        }
    });

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(documentationButton);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(sourceButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(themeButton);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(backtrackButton);
    buttonLayout->addSpacing(15);

    // 描述文本
    ElaText* descText = new ElaText(this);
    descText->setText(desText);
    descText->setTextPixelSize(13);

    // 组装布局
    QVBoxLayout* topLayout = new QVBoxLayout(customWidget);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(subTitleText);
    topLayout->addSpacing(5);
    topLayout->addLayout(buttonLayout);
    topLayout->addSpacing(5);
    topLayout->addWidget(descText);

    setCustomWidget(customWidget);
}
