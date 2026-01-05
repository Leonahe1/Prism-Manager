#ifndef PROCESSSTATUSINDICATOR_H
#define PROCESSSTATUSINDICATOR_H

#include <QWidget>
#include <QTimer>
#include <QColor>

namespace Prism {

/**
 * @brief 进程状态指示灯组件
 *
 * 圆形指示灯，支持 5 种状态：
 * - Stopped: 灰色，静态
 * - Starting: 黄色，闪烁
 * - Running: 绿色，闪烁
 * - Error: 红色，静态
 * - Killed: 橙色，静态
 */
class ProcessStatusIndicator : public QWidget
{
    Q_OBJECT

public:
    enum class Status {
        Stopped,    // 已停止 - 灰色
        Starting,   // 启动中 - 黄色闪烁
        Running,    // 运行中 - 绿色闪烁
        Error,      // 错误退出 - 红色
        Killed      // 强制终止 - 橙色
    };

    explicit ProcessStatusIndicator(QWidget* parent = nullptr);
    ~ProcessStatusIndicator() override;

    /**
     * @brief 设置状态
     */
    void setStatus(Status status);

    /**
     * @brief 获取当前状态
     */
    Status status() const { return m_status; }

    /**
     * @brief 获取状态对应的中文名称
     */
    static QString statusName(Status status);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onBlinkTimeout();

private:
    void startBlinking();
    void stopBlinking();
    QColor getStatusColor() const;

    Status m_status;
    QTimer* m_blinkTimer;
    bool m_blinkVisible;     // 闪烁状态：true=显示，false=隐藏
    int m_indicatorSize;     // 指示灯直径
};

} // namespace Prism

#endif // PROCESSSTATUSINDICATOR_H
