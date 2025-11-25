#pragma once

#include <QString>
#include <QVariantMap>
#include <memory>

namespace Prism {

// 配置格式枚举
enum class ConfigFormat {
    Auto,       // 自动检测
    INI,
    JSON,
    YAML,
    XML,
    Unknown
};

// 配置解析器基类
class ConfigParser {
public:
    virtual ~ConfigParser() = default;

    /**
     * @brief 解析配置文件
     * @param filePath 文件路径
     * @return 解析后的配置数据（键值对）
     */
    virtual QVariantMap parse(const QString& filePath) = 0;

    /**
     * @brief 保存配置到文件
     * @param filePath 文件路径
     * @param data 配置数据
     * @return 是否成功
     */
    virtual bool save(const QString& filePath, const QVariantMap& data) = 0;

    /**
     * @brief 验证文件格式是否匹配
     * @param filePath 文件路径
     * @return 是否匹配
     */
    virtual bool canParse(const QString& filePath) const = 0;

    /**
     * @brief 获取解析器支持的格式
     * @return 格式类型
     */
    virtual ConfigFormat getFormat() const = 0;

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息
     */
    QString getLastError() const { return m_lastError; }

protected:
    QString m_lastError;
};

// 智能指针别名
using ConfigParserPtr = std::shared_ptr<ConfigParser>;

} // namespace Prism
