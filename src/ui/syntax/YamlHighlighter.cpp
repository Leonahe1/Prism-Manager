#include "YamlHighlighter.h"

namespace Prism {

YamlHighlighter::YamlHighlighter(QTextDocument* parent)
    : SyntaxHighlighter(parent)
{
    updateColors();
    setupRules();
}

void YamlHighlighter::updateColors()
{
    if (m_isDarkTheme) {
        // 深色主题颜色
        m_keyFormat.setForeground(QColor("#61AFEF"));        // 蓝色
        m_stringFormat.setForeground(QColor("#98C379"));     // 绿色
        m_numberFormat.setForeground(QColor("#D19A66"));     // 橙色
        m_boolFormat.setForeground(QColor("#C678DD"));       // 紫色
        m_commentFormat.setForeground(QColor("#5C6370"));    // 灰色
        m_commentFormat.setFontItalic(true);
        m_listMarkerFormat.setForeground(QColor("#E5C07B")); // 黄色
        m_listMarkerFormat.setFontWeight(QFont::Bold);
        m_nullFormat.setForeground(QColor("#E06C75"));       // 红色
    } else {
        // 浅色主题颜色
        m_keyFormat.setForeground(QColor("#0078D4"));        // 蓝色
        m_stringFormat.setForeground(QColor("#107C10"));     // 绿色
        m_numberFormat.setForeground(QColor("#D83B01"));     // 橙色
        m_boolFormat.setForeground(QColor("#8764B8"));       // 紫色
        m_commentFormat.setForeground(QColor("#6A737D"));    // 灰色
        m_commentFormat.setFontItalic(true);
        m_listMarkerFormat.setForeground(QColor("#F7630C")); // 橙色
        m_listMarkerFormat.setFontWeight(QFont::Bold);
        m_nullFormat.setForeground(QColor("#D13438"));       // 红色
    }
}

void YamlHighlighter::setupRules()
{
    m_rules.clear();

    HighlightRule rule;

    // Key: key: (行首或缩进后的键)
    rule.pattern = QRegularExpression(R"(^\s*[^#\-\s][^:#]*(?=\s*:))");
    rule.format = m_keyFormat;
    m_rules.append(rule);

    // String: "value" or 'value'
    rule.pattern = QRegularExpression(R"("([^"\\]|\\.)*"|'([^'\\]|\\.)*')");
    rule.format = m_stringFormat;
    m_rules.append(rule);

    // Number: integers and floats
    rule.pattern = QRegularExpression(R"((?<=:\s)-?\d+\.?\d*(?:[eE][+-]?\d+)?(?=\s*$|\s*#))");
    rule.format = m_numberFormat;
    m_rules.append(rule);

    // Number in list
    rule.pattern = QRegularExpression(R"((?<=-\s)-?\d+\.?\d*(?:[eE][+-]?\d+)?(?=\s*$|\s*#))");
    rule.format = m_numberFormat;
    m_rules.append(rule);

    // Boolean: true/false/yes/no/on/off
    rule.pattern = QRegularExpression(R"(\b(true|false|yes|no|on|off|True|False|Yes|No|On|Off|TRUE|FALSE|YES|NO|ON|OFF)\b)");
    rule.format = m_boolFormat;
    m_rules.append(rule);

    // Null: null/~
    rule.pattern = QRegularExpression(R"(\b(null|Null|NULL|~)\b)");
    rule.format = m_nullFormat;
    m_rules.append(rule);

    // List marker: -
    rule.pattern = QRegularExpression(R"(^\s*-\s)");
    rule.format = m_listMarkerFormat;
    m_rules.append(rule);

    // Comment: #comment (必须放在最后以覆盖其他规则)
    rule.pattern = QRegularExpression(R"(#.*)");
    rule.format = m_commentFormat;
    m_rules.append(rule);
}

} // namespace Prism
