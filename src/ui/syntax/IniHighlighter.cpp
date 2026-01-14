#include "IniHighlighter.h"

namespace Prism {

IniHighlighter::IniHighlighter(QTextDocument* parent)
    : SyntaxHighlighter(parent)
{
    updateColors();
    setupRules();
}

void IniHighlighter::updateColors()
{
    if (m_isDarkTheme) {
        // 深色主题颜色
        m_sectionFormat.setForeground(QColor("#56B6C2"));    // 青色
        m_sectionFormat.setFontWeight(QFont::Bold);
        m_keyFormat.setForeground(QColor("#61AFEF"));        // 蓝色
        m_valueFormat.setForeground(QColor("#98C379"));      // 绿色
        m_commentFormat.setForeground(QColor("#5C6370"));    // 灰色
        m_commentFormat.setFontItalic(true);
    } else {
        // 浅色主题颜色
        m_sectionFormat.setForeground(QColor("#00979C"));    // 青色
        m_sectionFormat.setFontWeight(QFont::Bold);
        m_keyFormat.setForeground(QColor("#0078D4"));        // 蓝色
        m_valueFormat.setForeground(QColor("#107C10"));      // 绿色
        m_commentFormat.setForeground(QColor("#6A737D"));    // 灰色
        m_commentFormat.setFontItalic(true);
    }
}

void IniHighlighter::setupRules()
{
    m_rules.clear();

    HighlightRule rule;

    // Section: [section]
    rule.pattern = QRegularExpression(R"(^\s*\[[^\]]+\])");
    rule.format = m_sectionFormat;
    m_rules.append(rule);

    // Key: key= (匹配等号前的部分)
    rule.pattern = QRegularExpression(R"(^\s*([^=;\[#\s][^=]*?)(?=\s*=))");
    rule.format = m_keyFormat;
    m_rules.append(rule);

    // Value: =value (匹配等号后的部分)
    rule.pattern = QRegularExpression(R"(=\s*(.*)$)");
    rule.format = m_valueFormat;
    m_rules.append(rule);

    // Comment: ;comment 或 #comment
    rule.pattern = QRegularExpression(R"((^|(?<=\s))[;#].*)");
    rule.format = m_commentFormat;
    m_rules.append(rule);
}

} // namespace Prism
