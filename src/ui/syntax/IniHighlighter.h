#pragma once

#include "SyntaxHighlighter.h"

namespace Prism {

/**
 * @brief INI 格式语法高亮器
 *
 * 高亮规则:
 * - Section: [section] - 蓝色/青色
 * - Key: key= - 默认色
 * - Value: =value - 绿色
 * - Comment: ;comment / #comment - 灰色
 */
class IniHighlighter : public SyntaxHighlighter
{
    Q_OBJECT

public:
    explicit IniHighlighter(QTextDocument* parent = nullptr);

protected:
    void setupRules() override;
    void updateColors() override;
};

} // namespace Prism
