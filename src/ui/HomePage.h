#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include "BasePage.h"

class ElaMenu;

/**
 * @brief Prism 主页面
 *
 * 展示应用程序的核心功能卡片和快速导航入口
 */
class HomePage : public BasePage
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit HomePage(QWidget* parent = nullptr);
    ~HomePage();

Q_SIGNALS:
    // 导航信号
    Q_SIGNAL void projectManagementNavigation();
    Q_SIGNAL void configFileNavigation();
    Q_SIGNAL void processMonitorNavigation();

protected:
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    ElaMenu* _homeMenu{ nullptr };
};

#endif // HOMEPAGE_H
