#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

namespace Prism {

/**
 * @brief 语法高亮器基类
 *
 * 为配置文件提供语法高亮功能，支持主题切换
 */
class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument* parent = nullptr);
    virtual ~SyntaxHighlighter() = default;

    /**
     * @brief 设置主题模式
     * @param isDark 是否为深色主题
     */
    void setTheme(bool isDark);

    /**
     * @brief 获取当前是否为深色主题
     */
    bool isDarkTheme() const { return m_isDarkTheme; }

protected:
    /**
     * @brief 高亮规则结构
     */
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    /**
     * @brief 重写高亮方法
     */
    void highlightBlock(const QString& text) override;

    /**
     * @brief 设置高亮规则（子类实现）
     */
    virtual void setupRules() = 0;

    /**
     * @brief 更新颜色方案（子类实现）
     */
    virtual void updateColors() = 0;

    // 高亮规则列表
    QVector<HighlightRule> m_rules;

    // 当前主题
    bool m_isDarkTheme = true;

    // 通用格式
    QTextCharFormat m_keyFormat;        // 键格式
    QTextCharFormat m_valueFormat;      // 值格式
    QTextCharFormat m_stringFormat;     // 字符串格式
    QTextCharFormat m_numberFormat;     // 数字格式
    QTextCharFormat m_boolFormat;       // 布尔格式
    QTextCharFormat m_commentFormat;    // 注释格式
    QTextCharFormat m_sectionFormat;    // 段落格式
    QTextCharFormat m_bracketFormat;    // 括号格式
    QTextCharFormat m_nullFormat;       // null 格式
};

} // namespace Prism
