#include "YamlParser.h"
#include <QFile>
#include <QFileInfo>

#ifdef YAML_CPP_AVAILABLE
#include <yaml-cpp/yaml.h>
#endif

namespace Prism {

QVariantMap YamlParser::parse(const QString& filePath) {
    QVariantMap result;
    m_lastError.clear();

#ifdef YAML_CPP_AVAILABLE
    try {
        YAML::Node root = YAML::LoadFile(filePath.toStdString());
        flattenNode(root, result);
    } catch (const YAML::Exception& e) {
        m_lastError = QString("YAML parse error: %1").arg(e.what());
    } catch (const std::exception& e) {
        m_lastError = QString("Error: %1").arg(e.what());
    }
#else
    m_lastError = "yaml-cpp not available";
#endif

    return result;
}

bool YamlParser::save(const QString& filePath, const QVariantMap& data) {
    m_lastError.clear();

#ifdef YAML_CPP_AVAILABLE
    try {
        YAML::Emitter out;
        out << YAML::BeginMap;

        for (auto it = data.constBegin(); it != data.constEnd(); ++it) {
            out << YAML::Key << it.key().toStdString();
            out << YAML::Value << variantToNode(it.value());
        }

        out << YAML::EndMap;

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_lastError = QString("Cannot write file: %1").arg(filePath);
            return false;
        }

        file.write(out.c_str());
        file.close();
        return true;
    } catch (const std::exception& e) {
        m_lastError = QString("YAML write error: %1").arg(e.what());
        return false;
    }
#else
    m_lastError = "yaml-cpp not available";
    return false;
#endif
}

bool YamlParser::canParse(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    return suffix == "yaml" || suffix == "yml";
}

#ifdef YAML_CPP_AVAILABLE
QVariant YamlParser::nodeToVariant(const YAML::Node& node) {
    if (node.IsScalar()) {
        std::string value = node.as<std::string>();
        return QString::fromStdString(value);
    } else if (node.IsSequence()) {
        QVariantList list;
        for (const auto& item : node) {
            list.append(nodeToVariant(item));
        }
        return list;
    } else if (node.IsMap()) {
        QVariantMap map;
        for (const auto& pair : node) {
            QString key = QString::fromStdString(pair.first.as<std::string>());
            map[key] = nodeToVariant(pair.second);
        }
        return map;
    }
    return QVariant();
}

YAML::Node YamlParser::variantToNode(const QVariant& variant) {
    YAML::Node node;

    if (variant.type() == QVariant::Map || variant.type() == QVariant::Hash) {
        QVariantMap map = variant.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            node[it.key().toStdString()] = variantToNode(it.value());
        }
    } else if (variant.type() == QVariant::List) {
        QVariantList list = variant.toList();
        for (const auto& item : list) {
            node.push_back(variantToNode(item));
        }
    } else {
        node = variant.toString().toStdString();
    }

    return node;
}

void YamlParser::flattenNode(const YAML::Node& node, QVariantMap& map, const QString& prefix) {
    if (node.IsMap()) {
        for (const auto& pair : node) {
            QString key = QString::fromStdString(pair.first.as<std::string>());
            QString fullKey = prefix.isEmpty() ? key : prefix + "." + key;

            if (pair.second.IsMap()) {
                flattenNode(pair.second, map, fullKey);
            } else {
                map[fullKey] = nodeToVariant(pair.second);
            }
        }
    }
}
#endif

} // namespace Prism
