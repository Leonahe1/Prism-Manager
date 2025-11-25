#pragma once

#include <QString>
#include <QByteArray>

namespace Prism {

/**
 * @brief 编码转换工具类
 * 提供常用的字符编码转换功能
 */
class EncodingUtils {
public:
    /**
     * @brief GBK 转 UTF-8
     * @param gbkData GBK 编码的字节数据
     * @return UTF-8 字符串
     */
    static QString gbkToUtf8(const QByteArray& gbkData);

    /**
     * @brief UTF-8 转 GBK
     * @param utf8String UTF-8 字符串
     * @return GBK 编码的字节数据
     */
    static QByteArray utf8ToGbk(const QString& utf8String);

    /**
     * @brief 自动检测并转换为 UTF-8
     * @param data 原始字节数据
     * @return UTF-8 字符串
     */
    static QString autoDetectAndConvert(const QByteArray& data);

    /**
     * @brief 检测字符串是否为 UTF-8 编码
     * @param data 字节数据
     * @return 是否为 UTF-8
     */
    static bool isUtf8(const QByteArray& data);

private:
    EncodingUtils() = delete;
};

} // namespace Prism
