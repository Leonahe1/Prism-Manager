#pragma once

#include "ConfigParser.h"
#include <QVariant>

namespace Prism {

/**
 * @brief INI 格式解析器
 * 手动解析 INI 文件，支持 UTF-8 编码和中文
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
    /**
     * @brief 解析值字符串，推断类型
     * @param value 值字符串
     * @return 推断类型后的 QVariant
     */
    QVariant parseValue(const QString& value);
};

} // namespace Prism
