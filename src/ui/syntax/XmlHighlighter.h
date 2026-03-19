#pragma once

#include "SyntaxHighlighter.h"

namespace Prism
{

/**
 * @brief XML 格式语法高亮器
 */
class XmlHighlighter : public SyntaxHighlighter
{
    Q_OBJECT

public:
    explicit XmlHighlighter(QTextDocument *parent = nullptr);

protected:
    void setupRules() override;
    void updateColors() override;

private:
    QTextCharFormat m_commentFormat;
    QTextCharFormat m_attrValueFormat;
    QTextCharFormat m_attrNameFormat;
    QTextCharFormat m_tagNameFormat;
    QTextCharFormat m_bracketFormat;
};

} // namespace Prism
