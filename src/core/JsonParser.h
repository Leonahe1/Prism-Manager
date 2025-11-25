#pragma once

#include "ConfigParser.h"
#include <QJsonDocument>
#include <QJsonObject>

namespace Prism {

/**
 * @brief JSON 格式解析器
 * 使用 Qt 的 QJsonDocument 实现
 */
class JsonParser : public ConfigParser {
public:
    JsonParser() = default;
    ~JsonParser() override = default;

    QVariantMap parse(const QString& filePath) override;
    bool save(const QString& filePath, const QVariantMap& data) override;
    bool canParse(const QString& filePath) const override;
    ConfigFormat getFormat() const override { return ConfigFormat::JSON; }

private:
    // 将 QJsonObject 转换为 QVariantMap（支持嵌套）
    QVariantMap jsonObjectToVariantMap(const QJsonObject& obj, const QString& prefix = QString());
};

} // namespace Prism
