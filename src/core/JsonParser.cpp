#include "JsonParser.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>

namespace Prism {

QVariantMap JsonParser::parse(const QString& filePath) {
    QVariantMap result;
    m_lastError.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file: %1").arg(filePath);
        return result;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = QString("JSON parse error: %1").arg(parseError.errorString());
        return result;
    }

    if (doc.isObject()) {
        result = jsonObjectToVariantMap(doc.object());
    } else {
        m_lastError = "JSON root must be an object";
    }

    return result;
}

bool JsonParser::save(const QString& filePath, const QVariantMap& data) {
    m_lastError.clear();

    QJsonObject obj = QJsonObject::fromVariantMap(data);
    QJsonDocument doc(obj);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot write file: %1").arg(filePath);
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool JsonParser::canParse(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return suffix == "json";
}

QVariantMap JsonParser::jsonObjectToVariantMap(const QJsonObject& obj, const QString& prefix) {
    QVariantMap result;

    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        QString key = prefix.isEmpty() ? it.key() : prefix + "." + it.key();
        QJsonValue value = it.value();

        if (value.isObject()) {
            // 递归处理嵌套对象
            QVariantMap nested = jsonObjectToVariantMap(value.toObject(), key);
            for (auto nestedIt = nested.constBegin(); nestedIt != nested.constEnd(); ++nestedIt) {
                result[nestedIt.key()] = nestedIt.value();
            }
        } else if (value.isArray()) {
            // 数组转换为 QVariantList
            result[key] = value.toArray().toVariantList();
        } else {
            // 基本类型
            result[key] = value.toVariant();
        }
    }

    return result;
}

} // namespace Prism
