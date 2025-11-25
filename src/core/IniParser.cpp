#include "IniParser.h"
#include <QFile>
#include <QFileInfo>

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

    QSettings settings(filePath, QSettings::IniFormat);

    // 清空现有内容
    settings.clear();

    // 写入数据
    for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
        settings.setValue(it.key(), it.value());
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
        QString fullKey = prefix.isEmpty() ? key : prefix + "/" + key;
        map[fullKey] = settings.value(key);
    }

    // 递归处理所有子组
    QStringList groups = settings.childGroups();
    for (const QString& group : groups) {
        settings.beginGroup(group);
        QString newPrefix = prefix.isEmpty() ? group : prefix + "/" + group;
        readAllKeys(settings, map, newPrefix);
        settings.endGroup();
    }
}

} // namespace Prism
