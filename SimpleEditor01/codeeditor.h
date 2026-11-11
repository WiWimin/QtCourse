#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

class LineNumberArea;

/**
 * 代码编辑器控件：
 * 从 QPlainTextEdit 派生，在左侧预留一条行号栏，绘制当前文档的行号。
 * 该控件在主界面中通过 Qt Designer 的“提升(promote)”操作替换了原生 QPlainTextEdit。
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor() override;

    // 行号栏宽度：根据总行数的位数动态计算
    int lineNumberAreaWidth() const;

    // 供行号栏控件回调，真正完成绘制
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    bool isLineNumberVisible() const { return m_lineNumberVisible; }
    void setLineNumberVisible(bool visible);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // 行数变化 -> 重新计算行号栏宽度
    void updateLineNumberAreaWidth(int newBlockCount);
    // 内容滚动/更新 -> 只重绘受影响区域
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *m_lineNumberArea;
    bool m_lineNumberVisible;
};

#endif // CODEEDITOR_H
