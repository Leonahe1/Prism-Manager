#include "ProcessStatusIndicator.h"
#include <QPainter>
#include <QPainterPath>

namespace Prism {

ProcessStatusIndicator::ProcessStatusIndicator(QWidget* parent)
    : QWidget(parent)
    , m_status(Status::Stopped)
    , m_blinkTimer(new QTimer(this))
    , m_blinkVisible(true)
    , m_indicatorSize(14)
{
    // 设置固定大小
    setFixedSize(m_indicatorSize + 8, m_indicatorSize + 8);

    // 连接闪烁定时器
    connect(m_blinkTimer, &QTimer::timeout, this, &ProcessStatusIndicator::onBlinkTimeout);
}

ProcessStatusIndicator::~ProcessStatusIndicator()
{
    stopBlinking();
}

void ProcessStatusIndicator::setStatus(Status status)
{
    if (m_status == status) {
        return;
    }

    m_status = status;
    m_blinkVisible = true;

    // 根据状态决定是否启用闪烁
    if (status == Status::Running || status == Status::Starting) {
        startBlinking();
    } else {
        stopBlinking();
    }

    update();
}

QString ProcessStatusIndicator::statusName(Status status)
{
    switch (status) {
    case Status::Stopped:
        return QStringLiteral("已停止");
    case Status::Starting:
        return QStringLiteral("启动中");
    case Status::Running:
        return QStringLiteral("运行中");
    case Status::Error:
        return QStringLiteral("错误");
    case Status::Killed:
        return QStringLiteral("已终止");
    default:
        return QStringLiteral("未知");
    }
}

QSize ProcessStatusIndicator::sizeHint() const
{
    return QSize(m_indicatorSize + 8, m_indicatorSize + 8);
}

QSize ProcessStatusIndicator::minimumSizeHint() const
{
    return sizeHint();
}

void ProcessStatusIndicator::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 计算圆心位置（居中）
    int x = (width() - m_indicatorSize) / 2;
    int y = (height() - m_indicatorSize) / 2;
    QRectF rect(x, y, m_indicatorSize, m_indicatorSize);

    QColor color = getStatusColor();

    // 闪烁效果：在隐藏状态时降低透明度
    if (!m_blinkVisible && (m_status == Status::Running || m_status == Status::Starting)) {
        color.setAlpha(60);
    }

    // 绘制外发光效果（可选）
    if (m_blinkVisible && (m_status == Status::Running || m_status == Status::Starting)) {
        QColor glowColor = color;
        glowColor.setAlpha(80);
        painter.setPen(Qt::NoPen);
        painter.setBrush(glowColor);
        painter.drawEllipse(rect.adjusted(-2, -2, 2, 2));
    }

    // 绘制主圆形
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(rect);

    // 绘制高光效果（使指示灯更立体）
    if (m_blinkVisible || m_status == Status::Stopped || m_status == Status::Error || m_status == Status::Killed) {
        QColor highlightColor = Qt::white;
        highlightColor.setAlpha(100);
        QRectF highlightRect(x + 2, y + 2, m_indicatorSize / 3, m_indicatorSize / 3);
        painter.setBrush(highlightColor);
        painter.drawEllipse(highlightRect);
    }
}

void ProcessStatusIndicator::onBlinkTimeout()
{
    m_blinkVisible = !m_blinkVisible;
    update();
}

void ProcessStatusIndicator::startBlinking()
{
    if (!m_blinkTimer->isActive()) {
        m_blinkVisible = true;
        m_blinkTimer->start(500);  // 500ms 间隔，1秒完成一个闪烁周期
    }
}

void ProcessStatusIndicator::stopBlinking()
{
    if (m_blinkTimer->isActive()) {
        m_blinkTimer->stop();
    }
    m_blinkVisible = true;
    update();
}

QColor ProcessStatusIndicator::getStatusColor() const
{
    switch (m_status) {
    case Status::Running:
        return QColor("#4CAF50");  // 绿色
    case Status::Starting:
        return QColor("#FFC107");  // 琥珀黄
    case Status::Stopped:
        return QColor("#757575");  // 中灰
    case Status::Error:
        return QColor("#F44336");  // 红色
    case Status::Killed:
        return QColor("#FF9800");  // 橙色
    default:
        return QColor("#757575");  // 默认灰色
    }
}

} // namespace Prism
