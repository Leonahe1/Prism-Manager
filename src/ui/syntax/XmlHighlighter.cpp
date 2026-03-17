#include "XmlHighlighter.h"

namespace Prism
{

XmlHighlighter::XmlHighlighter(QTextDocument *parent)
    : SyntaxHighlighter(parent)
{
    updateColors();
    setupRules();
}

void XmlHighlighter::setupRules()
{
    m_rules.clear();
    HighlightRule rule;

    // 1. Comments: <!-- -->
    rule.pattern = QRegularExpression(R"(<!--[\s\S]*?-->)");
    rule.format = m_commentFormat;
    m_rules.append(rule);

    // 2. Attribute values: "value"
    rule.pattern = QRegularExpression(R"("([^"\\]|\\.)*")");
    rule.format = m_attrValueFormat;
    m_rules.append(rule);

    // 3. Attribute names (words followed by =)
    rule.pattern = QRegularExpression("\\b([\\w-]+)(?=\\s*=)");
    rule.format = m_attrNameFormat;
    m_rules.append(rule);

    // 4. Tag names (words after < or </)
    rule.pattern = QRegularExpression("(?<=</?)[\\w-]+");
    rule.format = m_tagNameFormat;
    m_rules.append(rule);

    // 5. Tag brackets
    rule.pattern = QRegularExpression("</?|/?>");
    rule.format = m_bracketFormat;
    m_rules.append(rule);
}

void XmlHighlighter::updateColors()
{
    bool isDark = isDarkTheme();

    // Comments
    m_commentFormat = QTextCharFormat{};
    m_commentFormat.setForeground(isDark ? QColor("#5C6370") : QColor("#A0A1A7")); // Gray
    m_commentFormat.setFontItalic(true);

    // Attribute values
    m_attrValueFormat = QTextCharFormat{};
    m_attrValueFormat.setForeground(isDark ? QColor("#98C379") : QColor("#50A14F")); // Green

    // Attribute names
    m_attrNameFormat = QTextCharFormat{};
    m_attrNameFormat.setForeground(isDark ? QColor("#D19A66") : QColor("#986801")); // Orange

    // Tag names
    m_tagNameFormat = QTextCharFormat{};
    m_tagNameFormat.setForeground(isDark ? QColor("#E06C75") : QColor("#A62626")); // Red

    // Tag brackets
    m_bracketFormat = QTextCharFormat{};
    m_bracketFormat.setForeground(isDark ? QColor("#ABB2BF") : QColor("#383A42")); // Default text color
}

} // namespace Prism
