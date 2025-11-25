#pragma once

#include "ConfigParser.h"
#include <QSettings>

namespace Prism {

/**
 * @brief INI 格式解析器
 * 使用 Qt 的 QSettings 实现
 */
class IniParser : public ConfigParser {
public:
    IniParser() = default;
    ~IniParser() override = default;

    QVariantMap parse(const QString& filePath) override;
    bool save(const QString& filePath, const QVariantMap& data) override;
    bool canParse(const QString& filePath) const override;
    ConfigFormat getFormat() const override { return ConfigFormat::INI; }

private:
    // 递归读取 QSettings 的所有键值对
    void readAllKeys(QSettings& settings, QVariantMap& map, const QString& prefix = QString());
};

} // namespace Prism
