#include "IniParser.h"
#include <QFile>
#include <QFileInfo>
#include <qset.h>

namespace Prism {

QVariantMap IniParser::parse(const QString& filePath) {
    QVariantMap result;
    m_lastError.clear();

    if (!QFile::exists(filePath)) {
        m_lastError = QString("File not found: %1").arg(filePath);
        return result;
    }

    QSettings settings(filePath, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        m_lastError = "Failed to open INI file";
        return result;
    }

    // 读取所有键值对
    readAllKeys(settings, result);

    return result;
}

bool IniParser::save(const QString& filePath, const QVariantMap& data) {
    m_lastError.clear();

    // 步骤1: 先读取原始文件，收集所有section路径（包括带.的section如 [app.log]）
    QSet<QString> originalSections;
    if (QFile::exists(filePath)) {
        QSettings originalSettings(filePath, QSettings::IniFormat);

        // 递归收集所有section路径
        std::function<void(QSettings&, const QString&)> collectSections;
        collectSections = [&](QSettings& s, const QString& prefix) {
            QStringList groups = s.childGroups();
            for (const QString& group : groups) {
                QString fullPath = prefix.isEmpty() ? group : prefix + "." + group;
                originalSections.insert(fullPath);

                s.beginGroup(group);
                collectSections(s, fullPath);
                s.endGroup();
            }
        };
        collectSections(originalSettings, "");
    }

    // 步骤2: 创建新文件并写入
    QSettings settings(filePath, QSettings::IniFormat);
    settings.clear();

    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        QString key = it.key();

        // 查找最长匹配的section名称
        QString matchedSection;
        for (const QString& section : originalSections) {
            if (key.startsWith(section + ".") && section.length() > matchedSection.length()) {
                matchedSection = section;
            }
        }

        QString qsettingsKey;
        if (!matchedSection.isEmpty()) {
            // 提取section后的部分
            QString remainder = key.mid(matchedSection.length() + 1);
            // section名保持不变（包含.），remainder中的.替换为/
            qsettingsKey = matchedSection + "/" + remainder.replace('.', '/');
        } else {
            // 没有匹配的section，全部.替换为/
            qsettingsKey = key;
            qsettingsKey.replace('.', '/');
        }

        settings.setValue(qsettingsKey, it.value());
    }

    settings.sync();

    if (settings.status() != QSettings::NoError) {
        m_lastError = "Failed to save INI file";
        return false;
    }

    return true;
}

bool IniParser::canParse(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    // 支持常见的 INI 后缀
    return suffix == "ini" || suffix == "conf" || suffix == "cfg";
}

void IniParser::readAllKeys(QSettings& settings, QVariantMap& map, const QString& prefix) {
    // 获取当前组的所有键
    QStringList keys = settings.childKeys();
    for (const QString& key : keys) {
        QString fullKey = prefix.isEmpty() ? key : prefix + "." + key;  // 使用 "." 统一格式
        map[fullKey] = settings.value(key);
    }

    // 递归处理所有子组
    QStringList groups = settings.childGroups();
    for (const QString& group : groups) {
        settings.beginGroup(group);
        QString newPrefix = prefix.isEmpty() ? group : prefix + "." + group;  // 使用 "." 统一格式
        readAllKeys(settings, map, newPrefix);
        settings.endGroup();
    }
}

} // namespace Prism
