#include "ParserFactory.h"
#include "IniParser.h"
#include "JsonParser.h"
#include "YamlParser.h"
#include "XmlParser.h"
#include <QFileInfo>
#include <QDebug>

namespace Prism {

ParserFactory& ParserFactory::instance() {
    static ParserFactory factory;
    return factory;
}

ParserFactory::ParserFactory() {
    // 可以在这里预注册一些常见的自定义扩展名
    // 例如：m_customExtensions["gttc"] = ConfigFormat::INI;
}

QVariantMap ParserFactory::parse(const QString& filePath, ConfigFormat forcedFormat) {
    m_lastError.clear();
    QVariantMap result;

    // 确定使用的格式
    ConfigFormat format = forcedFormat;
    if (format == ConfigFormat::Auto) {
        format = detectFormat(filePath);
        if (format == ConfigFormat::Unknown) {
            m_lastError = QString("Cannot detect format for file: %1").arg(filePath);
            return result;
        }
    }

    // 创建对应的解析器
    ConfigParserPtr parser = createParser(format);
    if (!parser) {
        m_lastError = QString("No parser available for format: %1").arg(formatToString(format));
        return result;
    }

    // 执行解析
    result = parser->parse(filePath);
    if (result.isEmpty() && !parser->getLastError().isEmpty()) {
        m_lastError = parser->getLastError();
    }

    qDebug() << "[ParserFactory] Parsed" << filePath << "using" << formatToString(format)
             << "format, got" << result.size() << "keys";

    return result;
}

bool ParserFactory::save(const QString& filePath, const QVariantMap& data, ConfigFormat format) {
    m_lastError.clear();

    if (format == ConfigFormat::Auto || format == ConfigFormat::Unknown) {
        m_lastError = "Must specify a valid format for saving";
        return false;
    }

    ConfigParserPtr parser = createParser(format);
    if (!parser) {
        m_lastError = QString("No parser available for format: %1").arg(formatToString(format));
        return false;
    }

    bool success = parser->save(filePath, data);
    if (!success) {
        m_lastError = parser->getLastError();
    }

    return success;
}

ConfigParserPtr ParserFactory::createParser(ConfigFormat format) {
    switch (format) {
        case ConfigFormat::INI:
            return std::make_shared<IniParser>();
        case ConfigFormat::JSON:
            return std::make_shared<JsonParser>();
        case ConfigFormat::YAML:
            return std::make_shared<YamlParser>();
        case ConfigFormat::XML:
            return std::make_shared<XmlParser>();
        default:
            return nullptr;
    }
}

ConfigFormat ParserFactory::detectFormat(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    // 1. 首先检查自定义扩展名映射
    if (m_customExtensions.contains(suffix)) {
        return m_customExtensions[suffix];
    }

    // 2. 标准扩展名检测
    if (suffix == "ini" || suffix == "conf" || suffix == "cfg") {
        return ConfigFormat::INI;
    } else if (suffix == "json") {
        return ConfigFormat::JSON;
    } else if (suffix == "yaml" || suffix == "yml") {
        return ConfigFormat::YAML;
    } else if (suffix == "xml") {
        return ConfigFormat::XML;
    }

    // 3. 尝试通过文件内容检测（简单启发式）
    // 这里可以进一步扩展，读取文件头几个字节来判断
    // 例如：JSON 通常以 { 或 [ 开头，YAML 可能有 --- 等

    return ConfigFormat::Unknown;
}

void ParserFactory::registerCustomExtension(const QString& extension, ConfigFormat format) {
    QString ext = extension.toLower();
    if (ext.startsWith('.')) {
        ext = ext.mid(1); // 移除前导点
    }

    m_customExtensions[ext] = format;
    qDebug() << "[ParserFactory] Registered custom extension:" << ext << "->" << formatToString(format);
}

QString ParserFactory::formatToString(ConfigFormat format) {
    switch (format) {
        case ConfigFormat::Auto:    return "Auto";
        case ConfigFormat::INI:     return "INI";
        case ConfigFormat::JSON:    return "JSON";
        case ConfigFormat::YAML:    return "YAML";
        case ConfigFormat::XML:     return "XML";
        case ConfigFormat::Unknown: return "Unknown";
        default:                    return "Unknown";
    }
}

} // namespace Prism
