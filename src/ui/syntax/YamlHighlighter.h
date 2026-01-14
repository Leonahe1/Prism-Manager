#pragma once

#include "SyntaxHighlighter.h"

namespace Prism {

/**
 * @brief YAML 格式语法高亮器
 *
 * 高亮规则:
 * - Key: key: - 蓝色
 * - String: "value" / 'value' - 绿色
 * - Number: 123 - 橙色
 * - Boolean: true/false/yes/no - 紫色
 * - Comment: #comment - 灰色
 * - List marker: - - 黄色
 */
class YamlHighlighter : public SyntaxHighlighter
{
    Q_OBJECT

public:
    explicit YamlHighlighter(QTextDocument* parent = nullptr);

protected:
    void setupRules() override;
    void updateColors() override;

private:
    QTextCharFormat m_listMarkerFormat;  // 列表标记格式
};

} // namespace Prism
