#include "JsonHighlighter.h"

namespace Prism {

JsonHighlighter::JsonHighlighter(QTextDocument* parent)
    : SyntaxHighlighter(parent)
{
    updateColors();
    setupRules();
}

void JsonHighlighter::updateColors()
{
    if (m_isDarkTheme) {
        // 深色主题颜色
        m_keyFormat.setForeground(QColor("#61AFEF"));        // 蓝色
        m_stringFormat.setForeground(QColor("#98C379"));     // 绿色
        m_numberFormat.setForeground(QColor("#D19A66"));     // 橙色
        m_boolFormat.setForeground(QColor("#C678DD"));       // 紫色
        m_nullFormat.setForeground(QColor("#E06C75"));       // 红色
        m_bracketFormat.setForeground(QColor("#ABB2BF"));    // 灰白
    } else {
        // 浅色主题颜色
        m_keyFormat.setForeground(QColor("#0078D4"));        // 蓝色
        m_stringFormat.setForeground(QColor("#107C10"));     // 绿色
        m_numberFormat.setForeground(QColor("#D83B01"));     // 橙色
        m_boolFormat.setForeground(QColor("#8764B8"));       // 紫色
        m_nullFormat.setForeground(QColor("#D13438"));       // 红色
        m_bracketFormat.setForeground(QColor("#323130"));    // 深灰
    }
}

void JsonHighlighter::setupRules()
{
    m_rules.clear();

    HighlightRule rule;

    // Key: "key":
    rule.pattern = QRegularExpression(R"("([^"\\]|\\.)*"\s*:)");
    rule.format = m_keyFormat;
    m_rules.append(rule);

    // String value: "value" (不包含 key 后的冒号)
    rule.pattern = QRegularExpression(R"(:\s*"([^"\\]|\\.)*")");
    rule.format = m_stringFormat;
    m_rules.append(rule);

    // String in array: ["value", "value2"]
    rule.pattern = QRegularExpression(R"(\[\s*"([^"\\]|\\.)*")");
    rule.format = m_stringFormat;
    m_rules.append(rule);

    // String after comma in array
    rule.pattern = QRegularExpression(R"(,\s*"([^"\\]|\\.)*")");
    rule.format = m_stringFormat;
    m_rules.append(rule);

    // Number: integers and floats
    rule.pattern = QRegularExpression(R"((?<=[:\[,\s])-?\d+\.?\d*(?:[eE][+-]?\d+)?(?=[\s,\]\}]))");
    rule.format = m_numberFormat;
    m_rules.append(rule);

    // Boolean: true/false
    rule.pattern = QRegularExpression(R"(\b(true|false)\b)");
    rule.format = m_boolFormat;
    m_rules.append(rule);

    // Null: null
    rule.pattern = QRegularExpression(R"(\bnull\b)");
    rule.format = m_nullFormat;
    m_rules.append(rule);

    // Brackets: {}[]
    rule.pattern = QRegularExpression(R"([\{\}\[\]])");
    rule.format = m_bracketFormat;
    m_rules.append(rule);
}

} // namespace Prism
