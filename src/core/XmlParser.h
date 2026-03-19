#pragma once

#include "ConfigParser.h"
#include <QHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace Prism
{

/**
 * @brief XML 格式解析器 (流式实现)
 * 使用 QXmlStreamReader 和 QXmlStreamWriter
 */
class XmlParser : public ConfigParser
{
public:
    XmlParser() = default;
    ~XmlParser() override = default;

    QVariantMap parse(const QString &filePath) override;
    bool save(const QString &filePath, const QVariantMap &data) override;
    bool canParse(const QString &filePath) const override;
    ConfigFormat getFormat() const override
    {
        return ConfigFormat::XML;
    }

private:
    // 流式解析 XML 到扁平化 QVariantMap（键路径使用 "."）
    void readXml(QXmlStreamReader &reader, QVariantMap &result, const QString &prefix = QString());

    // 将扁平化的 QVariantMap 转换为嵌套的结构以便写入
    struct XmlNode
    {
        QString value;
        QMap<QString, XmlNode> children;
    };
    void buildXmlTree(const QVariantMap &data, XmlNode &root);
    void writeXmlNode(QXmlStreamWriter &writer, const QString &name, const XmlNode &node);

    // 用于将同名兄弟节点展开为数组：tag[0000]、tag[0001]...
    // key 为“未带索引的路径”，value 为下一个可用的 0-based 索引
    QHash<QString, int> m_nextIndexByPath;
};

} // namespace Prism
