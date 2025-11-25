#pragma once

#include "ConfigParser.h"
#include <QMap>
#include <QString>

namespace Prism {

/**
 * @brief 配置解析器工厂
 * 负责创建和管理不同格式的解析器
 * 支持自动检测和强制指定格式
 */
class ParserFactory {
public:
    // 获取单例
    static ParserFactory& instance();

    /**
     * @brief 解析配置文件（高层接口）
     * @param filePath 文件路径
     * @param forcedFormat 强制指定的格式（默认为 Auto 自动检测）
     * @return 解析后的配置数据
     */
    QVariantMap parse(const QString& filePath, ConfigFormat forcedFormat = ConfigFormat::Auto);

    /**
     * @brief 保存配置文件
     * @param filePath 文件路径
     * @param data 配置数据
     * @param format 保存格式（必须明确指定）
     * @return 是否成功
     */
    bool save(const QString& filePath, const QVariantMap& data, ConfigFormat format);

    /**
     * @brief 创建指定格式的解析器
     * @param format 格式类型
     * @return 解析器智能指针
     */
    ConfigParserPtr createParser(ConfigFormat format);

    /**
     * @brief 根据文件路径自动检测格式
     * @param filePath 文件路径
     * @return 检测到的格式
     */
    ConfigFormat detectFormat(const QString& filePath) const;

    /**
     * @brief 注册自定义文件扩展名映射
     * @param extension 文件扩展名（如 "gttc"）
     * @param format 对应的格式类型
     *
     * 示例：registerCustomExtension("gttc", ConfigFormat::INI);
     */
    void registerCustomExtension(const QString& extension, ConfigFormat format);

    /**
     * @brief 获取最后的错误信息
     * @return 错误信息
     */
    QString getLastError() const { return m_lastError; }

    /**
     * @brief 格式枚举转字符串
     */
    static QString formatToString(ConfigFormat format);

private:
    ParserFactory();
    ~ParserFactory() = default;

    // 禁用拷贝和赋值
    ParserFactory(const ParserFactory&) = delete;
    ParserFactory& operator=(const ParserFactory&) = delete;

    // 自定义扩展名映射表
    QMap<QString, ConfigFormat> m_customExtensions;

    QString m_lastError;
};

} // namespace Prism
