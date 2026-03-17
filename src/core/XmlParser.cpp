#include "XmlParser.h"
#include "utils/EncodingUtils.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>

namespace Prism {

QVariantMap XmlParser::parse(const QString& filePath) {
    QVariantMap result;
    m_lastError.clear();

    // 每次解析前重置同名节点索引计数
    m_nextIndexByPath.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Cannot open file: %1").arg(filePath);
        return result;
    }

    QByteArray data = file.readAll();
    file.close();

    // 自动检测编码并转换为 Unicode 字符串
    QString xmlContent = EncodingUtils::autoDetectAndConvert(data);

    QXmlStreamReader reader(xmlContent);
    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            // 解析 XML 树，保持与其他格式一致的扁平化键名
            readXml(reader, result, QString());
        }
    }

    if (reader.hasError()) {
        m_lastError = QString("XML parse error: %1 at line %2").arg(reader.errorString()).arg(reader.lineNumber());
    }

    return result;
}

void XmlParser::readXml(QXmlStreamReader& reader, QVariantMap& result, const QString& prefix) {
    const QString currentElement = reader.name().toString();

    // rawKey 为不带索引的路径，用来统计同名兄弟节点
    const QString rawKey = prefix.isEmpty() ? currentElement : prefix + "." + currentElement;

    // 0-based 递增索引，并用固定 4 位零填充，保证按字符串排序时也是数值顺序
    const int index = m_nextIndexByPath.value(rawKey, 0);
    m_nextIndexByPath.insert(rawKey, index + 1);
    const QString indexedKey = QString("%1[%2]").arg(rawKey).arg(index, 4, 10, QLatin1Char('0'));

    // 元素自身的 key 使用带索引版本，避免同名节点覆盖
    const QString baseKey = indexedKey;

    // 1) 先记录当前元素的属性：baseKey.@attrName
    const auto attrs = reader.attributes();
    for (const auto &attr : attrs) {
        const QString attrName = attr.name().toString();
        const QString attrKey = baseKey + ".@" + attrName;
        result[attrKey] = attr.value().toString();
    }

    // 2) 再处理子元素和文本
    while (!reader.atEnd() && !reader.hasError()) {
        const QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::Characters) {
            if (!reader.isWhitespace()) {
                // 纯文本内容直接落到元素键上
                result[baseKey] = reader.text().toString();
            }
        } else if (token == QXmlStreamReader::StartElement) {
            // 递归处理子元素
            readXml(reader, result, baseKey);
        } else if (token == QXmlStreamReader::EndElement) {
            break;
        }
    }
}

bool XmlParser::save(const QString &filePath, const QVariantMap &data)
{
    m_lastError.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_lastError = QString("Cannot write file: %1").arg(filePath);
        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("config");

    // 构建树形结构以支持嵌套写入
    XmlNode root;
    buildXmlTree(data, root);

    for (auto it = root.children.begin(); it != root.children.end(); ++it)
    {
        writeXmlNode(writer, it.key(), it.value());
    }

    writer.writeEndElement();
    writer.writeEndDocument();
    file.close();

    return true;
}

void XmlParser::buildXmlTree(const QVariantMap &data, XmlNode &root)
{
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        QStringList parts = it.key().split('.');
        XmlNode *current = &root;
        for (const QString &part : parts)
        {
            current = &current->children[part];
        }
        current->value = it.value().toString();
    }
}

void XmlParser::writeXmlNode(QXmlStreamWriter &writer, const QString &name, const XmlNode &node)
{
    writer.writeStartElement(name);
    if (!node.value.isEmpty())
    {
        writer.writeCharacters(node.value);
    }
    for (auto it = node.children.begin(); it != node.children.end(); ++it)
    {
        writeXmlNode(writer, it.key(), it.value());
    }
    writer.writeEndElement();
}

bool XmlParser::canParse(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower() == "xml";
}

} // namespace Prism
