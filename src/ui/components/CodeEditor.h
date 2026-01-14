#pragma once

#include <QPlainTextEdit>
#include <QWidget>

namespace Prism {

class LineNumberArea;

/**
 * @brief 带行号的代码编辑器
 *
 * 基于 QPlainTextEdit，添加行号区域和当前行高亮功能
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget* parent = nullptr);

    /**
     * @brief 绘制行号区域
     */
    void lineNumberAreaPaintEvent(QPaintEvent* event);

    /**
     * @brief 计算行号区域宽度
     */
    int lineNumberAreaWidth();

    /**
     * @brief 设置主题模式
     * @param isDark 是否为深色主题
     */
    void setTheme(bool isDark);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);

private:
    QWidget* m_lineNumberArea;
    bool m_isDarkTheme = true;

    // 主题颜色
    QColor m_lineNumberBgColor;
    QColor m_lineNumberTextColor;
    QColor m_currentLineBgColor;
};

/**
 * @brief 行号区域组件
 */
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor* editor)
        : QWidget(editor)
        , m_codeEditor(editor)
    {
    }

    QSize sizeHint() const override
    {
        return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor* m_codeEditor;
};

} // namespace Prism
