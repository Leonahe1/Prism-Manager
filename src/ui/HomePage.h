#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include "BasePage.h"
#include <QDateTime>
#include <QList>

class ElaMenu;
class ElaText;
class ElaPushButton;
class ElaIconButton;
class ElaScrollArea;
class QVBoxLayout;

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

    /**
     * @brief 刷新首页数据（从 MainWindow 获取最新数据）
     */
    void refreshData();

Q_SIGNALS:
    // 导航信号
    Q_SIGNAL void projectManagementNavigation();
    Q_SIGNAL void configFileNavigation();
    Q_SIGNAL void processMonitorNavigation();

protected:
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual bool eventFilter(QObject* obj, QEvent* event) override;
    virtual void showEvent(QShowEvent* event) override;

private slots:
    void onThemeChanged();
    void onProjectCardClicked(const QString& projectId);
    void onOpenConfigClicked();

private:
    void initUI();
    void setupConnections();
    void updateStatistics();
    void updateRecentProjects();
    void updateTheme();
    QString getCardStyleSheet() const;

    QWidget* createStatCard(const QString& title, const QString& value, int iconType);
    QWidget* createProjectCard(const QString& id, const QString& name,
                               const QString& path, const QDateTime& lastOpened,
                               int configCount);

    // ========================================
    // UI 组件 - 统计卡片区域
    // ========================================
    QWidget* _statsContainer{ nullptr };
    QWidget* _totalProjectsCard{ nullptr };
    QWidget* _totalConfigsCard{ nullptr };
    QWidget* _runningProcessCard{ nullptr };

    ElaText* _totalProjectsValue{ nullptr };
    ElaText* _totalConfigsValue{ nullptr };
    ElaText* _runningProcessValue{ nullptr };

    // ========================================
    // UI 组件 - 最近项目区域
    // ========================================
    QWidget* _recentProjectsPanel{ nullptr };  // 左侧面板
    ElaScrollArea* _recentProjectsScrollArea{ nullptr };
    QWidget* _recentProjectsContainer{ nullptr };
    QVBoxLayout* _recentProjectsLayout{ nullptr };
    ElaText* _emptyProjectsHint{ nullptr };
    QList<QWidget*> _projectCards;  // 项目卡片列表（用于主题更新）

    // ========================================
    // UI 组件 - 快捷操作区域
    // ========================================
    QWidget* _quickActionsContainer{ nullptr };
    ElaPushButton* _openConfigButton{ nullptr };

    ElaMenu* _homeMenu{ nullptr };

    bool _isInitialized{ false };
};

#endif // HOMEPAGE_H
