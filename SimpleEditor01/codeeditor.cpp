#include "codeeditor.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTextBlock>

// ============ 行号栏控件 ============
// 只负责尺寸与事件转发，真正的绘制逻辑在 CodeEditor 中，
// 这样可以方便地访问 firstVisibleBlock()/blockBoundingGeometry() 等接口。
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), m_editor(editor) {}

    QSize sizeHint() const override
    {
        return QSize(m_editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_editor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *m_editor;
};

// ============ CodeEditor ============
CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(nullptr)
    , m_lineNumberVisible(true)
{
    m_lineNumberArea = new LineNumberArea(this);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);

    updateLineNumberAreaWidth(0);
}

CodeEditor::~CodeEditor()
{
}

int CodeEditor::lineNumberAreaWidth() const
{
    if (!m_lineNumberVisible)
        return 0;

    int digits = 1;
    int maxBlocks = qMax(1, blockCount());
    while (maxBlocks >= 10)
    {
        maxBlocks /= 10;
        ++digits;
    }
    // 左右各留 4 像素空白，避免数字贴边
    return 8 + fontMetrics().width(QLatin1Char('9')) * digits;
}

void CodeEditor::setLineNumberVisible(bool visible)
{
    if (m_lineNumberVisible == visible)
        return;

    m_lineNumberVisible = visible;
    m_lineNumberArea->setVisible(visible);
    updateLineNumberAreaWidth(0);
    viewport()->update();
}

void CodeEditor::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
    // 用视口边距把左侧让出来给行号栏
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);        // 垂直滚动时整栏平移即可
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                        lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    // 行号栏底色比正文略深一点，形成清晰的视觉分界
    painter.fillRect(event->rect(), QColor(0xF2, 0xF3, 0xF5));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    int curLine = textCursor().blockNumber();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            // 当前行用蓝色加粗提示，其余行灰色
            painter.setPen(curLine == blockNumber ? QColor(0x1A, 0x73, 0xE8)
                                                  : QColor(0x8A, 0x8F, 0x98));
            painter.drawText(0, top, m_lineNumberArea->width() - 4,
                             fontMetrics().height(),
                             Qt::AlignRight, QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
