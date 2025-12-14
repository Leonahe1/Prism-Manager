#include "IniParser.h"
#include "utils/EncodingUtils.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

namespace Prism {

QVariantMap IniParser::parse(const QString& filePath) {
    QVariantMap result;
    m_lastError.clear();

    QFile file(filePath);
    if (!file.exists()) {
        m_lastError = QString("File not found: %1").arg(filePath);
        return result;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Cannot open file: %1").arg(filePath);
        return result;
    }

    // 读取原始字节数据，自动检测编码
    QByteArray rawData = file.readAll();
    file.close();

    // 自动检测编码并转换为 QString
    QString content = EncodingUtils::autoDetectAndConvert(rawData);

    // 手动解析 INI 文件
    QString currentSection;
    QStringList lines = content.split('\n');

    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();

        // 跳过空行和注释
        if (trimmedLine.isEmpty() || trimmedLine.startsWith(';') || trimmedLine.startsWith('#')) {
            continue;
        }

        // 检查是否是 section 标题
        if (trimmedLine.startsWith('[') && trimmedLine.endsWith(']')) {
            currentSection = trimmedLine.mid(1, trimmedLine.length() - 2);
            continue;
        }

        // 解析键值对
        int equalPos = trimmedLine.indexOf('=');
        if (equalPos > 0) {
            QString key = trimmedLine.left(equalPos).trimmed();
            QString value = trimmedLine.mid(equalPos + 1).trimmed();

            // 构建完整路径
            QString fullKey = currentSection.isEmpty() ? key : currentSection + "." + key;

            // 尝试推断值类型
            result[fullKey] = parseValue(value);
        }
    }

    qDebug() << "[IniParser] Parsed" << filePath << "got" << result.size() << "keys";
    return result;
}

QVariant IniParser::parseValue(const QString& value) {
    // 尝试解析为布尔值
    QString lowerValue = value.toLower();
    if (lowerValue == "true" || lowerValue == "yes" || lowerValue == "on") {
        return true;
    }
    if (lowerValue == "false" || lowerValue == "no" || lowerValue == "off") {
        return false;
    }

    // 尝试解析为整数
    bool isInt = false;
    int intValue = value.toInt(&isInt);
    if (isInt) {
        return intValue;
    }

    // 尝试解析为浮点数
    bool isDouble = false;
    double doubleValue = value.toDouble(&isDouble);
    if (isDouble && value.contains('.')) {
        return doubleValue;
    }

    // 检查是否是逗号分隔的列表（返回 QStringList）
    if (value.contains(',')) {
        QStringList list = value.split(',');
        QStringList trimmedList;
        for (const QString& item : list) {
            trimmedList.append(item.trimmed());
        }
        return trimmedList;
    }

    // 默认返回字符串
    return value;
}

bool IniParser::save(const QString& filePath, const QVariantMap& data) {
    m_lastError.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file for writing: %1").arg(filePath);
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // 按 section 分组
    QMap<QString, QMap<QString, QVariant>> sections;

    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        QString fullKey = it.key();
        QVariant value = it.value();

        int dotPos = fullKey.indexOf('.');
        QString section;
        QString key;

        if (dotPos != -1) {
            section = fullKey.left(dotPos);
            key = fullKey.mid(dotPos + 1);
        } else {
            section = "";  // 无 section
            key = fullKey;
        }

        sections[section][key] = value;
    }

    // 写入文件
    for (auto sectionIt = sections.constBegin(); sectionIt != sections.constEnd(); ++sectionIt) {
        QString section = sectionIt.key();
        const QMap<QString, QVariant>& keys = sectionIt.value();

        if (!section.isEmpty()) {
            out << "[" << section << "]\n";
        }

        for (auto keyIt = keys.constBegin(); keyIt != keys.constEnd(); ++keyIt) {
            QString key = keyIt.key();
            QVariant value = keyIt.value();

            QString valueStr;
            if (value.type() == QVariant::Bool) {
                valueStr = value.toBool() ? "true" : "false";
            } else if (value.type() == QVariant::StringList) {
                valueStr = value.toStringList().join(", ");
            } else if (value.type() == QVariant::List) {
                QStringList strList;
                for (const QVariant& item : value.toList()) {
                    strList.append(item.toString());
                }
                valueStr = strList.join(", ");
            } else {
                valueStr = value.toString();
            }

            out << key << " = " << valueStr << "\n";
        }

        out << "\n";
    }

    file.close();
    return true;
}

bool IniParser::canParse(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    // 支持常见的 INI 后缀
    return suffix == "ini" || suffix == "conf" || suffix == "cfg";
}

} // namespace Prism
