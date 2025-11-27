#ifndef BASEPAGE_H
#define BASEPAGE_H

#include <ElaScrollPage.h>

class QVBoxLayout;

/**
 * @brief 基础页面类
 *
 * 为 Prism 项目中的所有页面提供通用功能：
 * - 自动响应主题变更
 * - 提供统一的页面边距
 * - 可选的顶部自定义工具栏
 */
class BasePage : public ElaScrollPage
{
    Q_OBJECT
public:
    explicit BasePage(QWidget* parent = nullptr);
    ~BasePage() override;

protected:
    /**
     * @brief 创建页面顶部自定义控件
     * @param desText 页面描述文本
     */
    void createCustomWidget(QString desText);
};

#endif // BASEPAGE_H
