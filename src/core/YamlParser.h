#pragma once

#include "ConfigParser.h"

namespace YAML {
    class Node;
}

namespace Prism {

/**
 * @brief YAML 格式解析器
 * 使用 yaml-cpp 库实现
 */
class YamlParser : public ConfigParser {
public:
    YamlParser() = default;
    ~YamlParser() override = default;

    QVariantMap parse(const QString& filePath) override;
    bool save(const QString& filePath, const QVariantMap& data) override;
    bool canParse(const QString& filePath) const override;
    ConfigFormat getFormat() const override { return ConfigFormat::YAML; }

private:
    // 将 YAML Node 转换为 QVariant
    QVariant nodeToVariant(const YAML::Node& node);

    // 将 QVariant 转换为 YAML Node
    YAML::Node variantToNode(const QVariant& variant);

    // 递归展开 YAML 节点（支持嵌套键）
    void flattenNode(const YAML::Node& node, QVariantMap& map, const QString& prefix = QString());
};

} // namespace Prism
