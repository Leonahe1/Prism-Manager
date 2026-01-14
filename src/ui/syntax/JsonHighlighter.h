#pragma once

#include "SyntaxHighlighter.h"

namespace Prism {

/**
 * @brief JSON 格式语法高亮器
 *
 * 高亮规则:
 * - Key: "key": - 蓝色
 * - String: "value" - 绿色
 * - Number: 123 - 橙色
 * - Boolean: true/false - 紫色
 * - Null: null - 红色
 * - Brackets: {}[] - 默认色
 */
class JsonHighlighter : public SyntaxHighlighter
{
    Q_OBJECT

public:
    explicit JsonHighlighter(QTextDocument* parent = nullptr);

protected:
    void setupRules() override;
    void updateColors() override;
};

} // namespace Prism
