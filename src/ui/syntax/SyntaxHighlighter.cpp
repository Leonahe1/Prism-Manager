#include "SyntaxHighlighter.h"

namespace Prism {

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
}

void SyntaxHighlighter::setTheme(bool isDark)
{
    if (m_isDarkTheme != isDark) {
        m_isDarkTheme = isDark;
        updateColors();
        setupRules();
        rehighlight();
    }
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    for (const HighlightRule& rule : qAsConst(m_rules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

} // namespace Prism
