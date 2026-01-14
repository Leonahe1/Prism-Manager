#include "CodeEditor.h"

#include <QPainter>
#include <QTextBlock>

namespace Prism {

CodeEditor::CodeEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    m_lineNumberArea = new LineNumberArea(this);

    // 连接信号
    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    // 初始化主题
    setTheme(true);

    // 设置等宽字体
    QFont font("Consolas", 10);
    font.setStyleHint(QFont::Monospace);
    setFont(font);

    // 设置 Tab 宽度为 4 个空格
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

void CodeEditor::setTheme(bool isDark)
{
    m_isDarkTheme = isDark;

    if (isDark) {
        // 深色主题
        m_lineNumberBgColor = QColor("#21252B");
        m_lineNumberTextColor = QColor("#5C6370");
        m_currentLineBgColor = QColor("#2C323C");

        setStyleSheet(
            "QPlainTextEdit {"
            "  background-color: #282C34;"
            "  color: #ABB2BF;"
            "  border: none;"
            "  selection-background-color: #3E4451;"
            "}"
        );
    } else {
        // 浅色主题
        m_lineNumberBgColor = QColor("#F3F3F3");
        m_lineNumberTextColor = QColor("#6A737D");
        m_currentLineBgColor = QColor("#FFF8DC");

        setStyleSheet(
            "QPlainTextEdit {"
            "  background-color: #FFFFFF;"
            "  color: #24292E;"
            "  border: 1px solid #E1E4E8;"
            "  selection-background-color: #C8E1FF;"
            "}"
        );
    }

    // 重新绘制
    highlightCurrentLine();
    m_lineNumberArea->update();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // 至少显示 3 位数字的宽度
    digits = qMax(3, digits);

    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void CodeEditor::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(m_currentLineBgColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), m_lineNumberBgColor);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(m_lineNumberTextColor);

            // 当前行号高亮
            if (blockNumber == textCursor().blockNumber()) {
                painter.setPen(m_isDarkTheme ? QColor("#ABB2BF") : QColor("#24292E"));
                QFont boldFont = painter.font();
                boldFont.setBold(true);
                painter.setFont(boldFont);
            } else {
                QFont normalFont = painter.font();
                normalFont.setBold(false);
                painter.setFont(normalFont);
            }

            painter.drawText(0, top, m_lineNumberArea->width() - 5, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

} // namespace Prism
