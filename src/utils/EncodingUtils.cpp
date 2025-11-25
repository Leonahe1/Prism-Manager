#include "EncodingUtils.h"
#include <QTextCodec>

namespace Prism {

QString EncodingUtils::gbkToUtf8(const QByteArray& gbkData) {
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    if (codec) {
        return codec->toUnicode(gbkData);
    }
    return QString::fromUtf8(gbkData); // 失败则尝试直接当作 UTF-8
}

QByteArray EncodingUtils::utf8ToGbk(const QString& utf8String) {
    QTextCodec* codec = QTextCodec::codecForName("GBK");
    if (codec) {
        return codec->fromUnicode(utf8String);
    }
    return utf8String.toUtf8(); // 失败则返回 UTF-8
}

QString EncodingUtils::autoDetectAndConvert(const QByteArray& data) {
    // 先尝试判断是否为有效的 UTF-8
    if (isUtf8(data)) {
        return QString::fromUtf8(data);
    }

    // 不是 UTF-8，尝试作为 GBK 解码（Windows 常见）
#ifdef Q_OS_WIN
    return gbkToUtf8(data);
#else
    // Linux/macOS 默认 UTF-8
    return QString::fromLocal8Bit(data);
#endif
}

bool EncodingUtils::isUtf8(const QByteArray& data) {
    // 简单的 UTF-8 验证
    int i = 0;
    int size = data.size();

    while (i < size) {
        unsigned char c = static_cast<unsigned char>(data[i]);

        if (c <= 0x7F) {
            // ASCII 字符
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            // 2 字节序列
            if (i + 1 >= size || (data[i + 1] & 0xC0) != 0x80) {
                return false;
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3 字节序列
            if (i + 2 >= size || (data[i + 1] & 0xC0) != 0x80 || (data[i + 2] & 0xC0) != 0x80) {
                return false;
            }
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4 字节序列
            if (i + 3 >= size || (data[i + 1] & 0xC0) != 0x80 ||
                (data[i + 2] & 0xC0) != 0x80 || (data[i + 3] & 0xC0) != 0x80) {
                return false;
            }
            i += 4;
        } else {
            return false;
        }
    }

    return true;
}

} // namespace Prism
