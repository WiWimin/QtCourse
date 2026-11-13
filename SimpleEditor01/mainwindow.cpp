#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "dialog.h"

#include <QVBoxLayout>
#include <QFontDialog>
#include <QColorDialog>
#include <QTextCharFormat>
#include <QPalette>
#include <QLabel>
#include <QTimer>
#include <QTextCursor>
#include <QTextDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_findDialog(nullptr)
    , m_replaceDialog(nullptr)
{
    ui->setupUi(this);

    // 获取centralwidget上的垂直布局
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    if (layout)
    {
        layout->setContentsMargins(0, 0, 0, 0); // 左、上、右、下全部清零，贴边铺满
        layout->setSpacing(0);
    }

    // 工具栏图标统一为 26px，只用图标不显示文字，三组按钮之间由分隔线区分
    ui->toolBar->setIconSize(QSize(26, 26));
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // 初始化状态：无文件、未修改
    m_curFilePath = "";
    m_isModified = false;

    // 状态栏右侧常驻信息：长度/行数/光标行列号
    m_statusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_statusLabel);

    // 自动换行、显示行号默认打开，与 UI 中 Action 的 checked 保持一致
    ui->action_Wrap->setChecked(true);
    ui->action_ShowLine->setChecked(true);
    on_action_Wrap_toggled(true);
    on_action_ShowLine_toggled(true);

    // 更新初始标题
    updateWindowTitle();
    statusBar()->showMessage("就绪");
    updateStatusBar();

    // copyAvailable 信号：选中文本改变时触发
    connect(ui->plainTextEdit,&QPlainTextEdit::copyAvailable,
            this,&MainWindow::on_plainTextEdit_copyAvailable);

    //  QPlainTextEdit原生信号，控制撤销、重做按钮可用状态
    connect(ui->plainTextEdit, &QPlainTextEdit::undoAvailable,
            this, &MainWindow::onUndoAvailable);
    connect(ui->plainTextEdit, &QPlainTextEdit::redoAvailable,
            this, &MainWindow::onRedoAvailable);

    //光标位置改变，更新状态栏
    connect(ui->plainTextEdit,&QPlainTextEdit::cursorPositionChanged,
            this,&MainWindow::updateStatusBar);
    //文本内容改变（新增/删除文字，字符总数、行数变化）
    connect(ui->plainTextEdit,&QPlainTextEdit::textChanged,
            this,&MainWindow::updateStatusBar);

    // 构造函数末尾添加初始状态
    ui->action_Undo->setEnabled(false);
    ui->action_Redo->setEnabled(false);
    ui->action_Cut->setEnabled(false);
    ui->action_Copy->setEnabled(false);

    // 定时自动保存（选做）：每5分钟在文件所在目录写一份 .autosave 副本
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(5 * 60 * 1000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSaveTimeout);
    m_autoSaveTimer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateWindowTitle()
{
    QString title;
    if (m_curFilePath.isEmpty())
    {
        // 新建未保存
        if (m_isModified)
            title = "*新建文本文件 - 简易文本编辑器";
        else
            title = "新建文本文件 - 简易文本编辑器";
    }
    else
    {
        // 已有文件路径，提取文件名
        QFileInfo info(m_curFilePath);
        QString fileName = info.fileName();
        if (m_isModified)
            title = "*" + fileName + " - 简易文本编辑器";
        else
            title = fileName + " - 简易文本编辑器";
    }
    this->setWindowTitle(title);
}

void MainWindow::on_plainTextEdit_textChanged()
{
    // 防止重复设置，可加判断优化，不加也能跑
    if (!m_isModified)
    {
        m_isModified = true;
        updateWindowTitle();
    }
}

bool MainWindow::maybeSave()
{
    // 没有修改，直接放行
    if (!m_isModified)
        return true;

    QMessageBox::StandardButton ret;
    ret = QMessageBox::question(this,
                                "提示",
                                "文件已修改，是否保存更改？",
                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save)
    {
        bool ok;
        if(m_curFilePath.isEmpty())
        {
            // 未保存过，弹出另存为
            QString path = QFileDialog::getSaveFileName(this,"另存为");
            if(path.isEmpty())
                return false;
            // 先记下路径再写文件，保证写入后标题立即使用新文件名
            m_curFilePath = path;
            ok = saveFile(path);
        }
        else
        {
            ok = saveFile(m_curFilePath);
        }
        return ok;
    }
    else if (ret == QMessageBox::Discard)
    {
        // 不保存，直接放行
        return true;
    }
    else
    {
        // 取消，终止操作
        return false;
    }
}

bool MainWindow::loadFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "打开失败", "打开文件失败，文件不存在或者权限不足");
        return false;
    }
    QByteArray content = file.readAll();
    file.close();
    // 读到编辑器（setPlainText 会同时清空撤销栈，避免读到新文件还能撤销出旧内容）
    ui->plainTextEdit->setPlainText(QString::fromUtf8(content));
    m_curFilePath = filePath;
    m_isModified = false;
    updateWindowTitle();
    statusBar()->showMessage("已打开：" + filePath, 5000);
    return true;
}

bool MainWindow::saveFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "保存失败", "无法写入文件，请检查权限");
        return false;
    }
    QString text = ui->plainTextEdit->toPlainText();
    file.write(text.toUtf8());
    file.close();
    m_isModified = false;
    updateWindowTitle();
    statusBar()->showMessage("保存成功：" + filePath, 5000);
    return true;
}

void MainWindow::on_action_New_triggered()
{
    // 先检查是否需要保存
    if (!maybeSave())
        return;
    // 用 setPlainText("") 而不是 clear()：clear() 会进撤销栈，新建后还能撤销出旧内容
    ui->plainTextEdit->setPlainText("");
    m_curFilePath = "";
    m_isModified = false;
    updateWindowTitle();
    statusBar()->showMessage("新建空白文档");
}

void MainWindow::on_action_Open_triggered()
{
    if (!maybeSave())
        return;
    QString filePath = QFileDialog::getOpenFileName(this, "打开文件");
    if (filePath.isEmpty())
        return;
    loadFile(filePath);
}

void MainWindow::on_action_Save_triggered()
{
    if (m_curFilePath.isEmpty())
    {
        // 无路径，跳转另存为
        on_action_SaveAs_triggered();
    }
    else
    {
        saveFile(m_curFilePath);
    }
}

void MainWindow::on_action_SaveAs_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, "另存为");
    if (filePath.isEmpty())
        return;
    // 先更新路径再保存，避免保存后标题仍显示“新建文本文件”
    m_curFilePath = filePath;
    saveFile(filePath);
}

void MainWindow::on_action_Quit_triggered()
{
    close(); // 调用close，会自动进入closeEvent
}

void MainWindow::on_action_About_triggered()
{
    Dialog dlg(this);
    dlg.exec();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
    {
        event->accept(); // 允许关闭
    }
    else
    {
        event->ignore(); // 用户点取消，阻止窗口关闭
    }
}

// 剪切、复制按钮使能控制
void MainWindow::on_plainTextEdit_copyAvailable(bool flag)
{
    // flag=true：有选中文本，剪切复制可用；false置灰
    ui->action_Cut->setEnabled(flag);
    ui->action_Copy->setEnabled(flag);
}

// 撤销按钮状态
void MainWindow::onUndoAvailable(bool canUndoFlag)
{
    ui->action_Undo->setEnabled(canUndoFlag);
}

// 重做按钮状态
void MainWindow::onRedoAvailable(bool canRedoFlag)
{
    ui->action_Redo->setEnabled(canRedoFlag);
}

// 自动换行：toggled会传入勾选状态checked
void MainWindow::on_action_Wrap_toggled(bool checked)
{
    if(checked)
    {
        // 根据控件宽度自动换行
        ui->plainTextEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    }
    else
    {
        // 不换行，横向滚动
        ui->plainTextEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    }
}

// 显示行号：控制派生控件左侧行号栏的可见性
void MainWindow::on_action_ShowLine_toggled(bool checked)
{
    ui->plainTextEdit->setLineNumberVisible(checked);
}

// 字体：修改整个编辑器字体
void MainWindow::on_action_Font_triggered()
{
    bool ok;
    // 弹出字体对话框，默认取编辑器当前字体
    QFont currentFont = ui->plainTextEdit->font();
    QFont newFont = QFontDialog::getFont(&ok, currentFont, this, "选择字体");
    if(ok)
    {
        ui->plainTextEdit->setFont(newFont);
    }
}

// 字体颜色：修改【选中文字】的前景颜色
void MainWindow::on_action_FontColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::black, this, "选择字体颜色");
    if(!color.isValid())
        return; // 用户点取消，直接退出

    QTextCharFormat fmt;
    fmt.setForeground(color);
    // 应用到选中文字
    ui->plainTextEdit->mergeCurrentCharFormat(fmt);
}

// 字体背景色：修改【选中文字】的背景底色
void MainWindow::on_action_SelBgColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::yellow, this, "选择文字背景色");
    if(!color.isValid())
        return;

    QTextCharFormat fmt;
    fmt.setBackground(color);
    ui->plainTextEdit->mergeCurrentCharFormat(fmt);
}

// 编辑器背景色：修改整个编辑区域背景（使用QPalette调色板）
void MainWindow::on_action_EditBgColor_triggered()
{
    QColor bgColor = QColorDialog::getColor(Qt::white, this, "选择编辑器背景颜色");
    if(!bgColor.isValid())
        return;

    QPalette pal = ui->plainTextEdit->palette();
    // QPalette::Base：文本编辑控件的文档背景色
    pal.setColor(QPalette::Base, bgColor);
    ui->plainTextEdit->setPalette(pal);
}

// ================= 查找与替换 =================
void MainWindow::on_action_Find_triggered()
{
    if (!m_findDialog)
    {
        m_findDialog = new FindDialog(this);
        connect(m_findDialog, &FindDialog::findNextRequested,
                this, &MainWindow::onFindNextRequested);
    }
    // 有选中内容时，直接把选中文字带进查找框，省去重复输入
    QString selected = ui->plainTextEdit->textCursor().selectedText();
    if (!selected.isEmpty() && !selected.contains(QChar::ParagraphSeparator))
        m_findDialog->setFindText(selected);

    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::on_action_Replace_triggered()
{
    if (!m_replaceDialog)
    {
        m_replaceDialog = new ReplaceDialog(this);
        connect(m_replaceDialog, &ReplaceDialog::findNextRequested,
                this, &MainWindow::onFindNextRequested);
        connect(m_replaceDialog, &ReplaceDialog::replaceRequested,
                this, &MainWindow::onReplaceRequested);
        connect(m_replaceDialog, &ReplaceDialog::replaceAllRequested,
                this, &MainWindow::onReplaceAllRequested);
    }
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

void MainWindow::onFindNextRequested()
{
    if (sender() == m_findDialog && m_findDialog)
        doFindNext(m_findDialog->findText(),
                   m_findDialog->isCaseSensitive(),
                   m_findDialog->isBackward());
    else if (sender() == m_replaceDialog && m_replaceDialog)
        doFindNext(m_replaceDialog->findText(),
                   m_replaceDialog->isCaseSensitive(),
                   m_replaceDialog->isBackward());
}

void MainWindow::doFindNext(const QString &text, bool caseSensitive, bool backward)
{
    if (text.isEmpty())
    {
        statusBar()->showMessage("请输入要查找的内容", 3000);
        return;
    }

    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    if (backward)
        flags |= QTextDocument::FindBackward;

    // 先按当前光标位置查找
    bool found = ui->plainTextEdit->find(text, flags);

    // 未找到则从另一端重新开始，实现“循环查找”
    if (!found)
    {
        QTextCursor cur = ui->plainTextEdit->textCursor();
        cur.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        ui->plainTextEdit->setTextCursor(cur);
        found = ui->plainTextEdit->find(text, flags);
    }

    if (found)
    {
        ui->plainTextEdit->centerCursor();
        statusBar()->showMessage(QString("已找到“%1”").arg(text), 3000);
    }
    else
    {
        statusBar()->showMessage(QString("未找到“%1”").arg(text), 3000);
    }
}

void MainWindow::onReplaceRequested()
{
    if (!m_replaceDialog)
        return;

    QString findText = m_replaceDialog->findText();
    QString replaceText = m_replaceDialog->replaceText();
    bool caseSensitive = m_replaceDialog->isCaseSensitive();

    if (findText.isEmpty())
    {
        statusBar()->showMessage("请输入要查找的内容", 3000);
        return;
    }

    // 若当前选中的内容正好是要查找的文字，就先替换，再继续查找下一个
    QTextCursor cur = ui->plainTextEdit->textCursor();
    QString selected = cur.selectedText();
    bool matched = !selected.isEmpty()
            && (caseSensitive ? selected == findText
                              : selected.compare(findText, Qt::CaseInsensitive) == 0);

    if (matched)
    {
        cur.insertText(replaceText);
        ui->plainTextEdit->setTextCursor(cur);
        statusBar()->showMessage("已替换 1 处", 3000);
    }

    doFindNext(findText, caseSensitive, m_replaceDialog->isBackward());
}

void MainWindow::onReplaceAllRequested()
{
    if (!m_replaceDialog)
        return;

    QString findText = m_replaceDialog->findText();
    QString replaceText = m_replaceDialog->replaceText();
    bool caseSensitive = m_replaceDialog->isCaseSensitive();

    if (findText.isEmpty())
    {
        statusBar()->showMessage("请输入要查找的内容", 3000);
        return;
    }

    int replaced = doReplaceAll(findText, replaceText, caseSensitive);
    if (replaced == 0)
        statusBar()->showMessage(QString("未找到“%1”，未做任何替换").arg(findText), 4000);
    else
        statusBar()->showMessage(QString("全部替换完成，共替换 %1 处").arg(replaced), 4000);
}

int MainWindow::doReplaceAll(const QString &findText, const QString &replaceText, bool caseSensitive)
{
    QTextDocument *doc = ui->plainTextEdit->document();

    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    int count = 0;
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::Start);

    QTextCursor found = doc->find(findText, cursor, flags);
    while (!found.isNull() && count < 100000)
    {
        found.insertText(replaceText);   // 替换当前匹配，光标停在替换文本之后
        ++count;
        found = doc->find(findText, found, flags);
    }
    return count;
}

void MainWindow::updateStatusBar()
{
    QTextCursor cur = ui->plainTextEdit->textCursor();
    int len = ui->plainTextEdit->toPlainText().length();
    int lines = ui->plainTextEdit->blockCount();
    int curLine = cur.blockNumber()+1;   // blockNumber从0开始，+1变成从1开始行号
    int curCol = cur.positionInBlock()+1;// 列从1开始

    // 统计信息放在状态栏右侧常驻，避免与左侧临时提示互相覆盖
    m_statusLabel->setText(
        QString("length:%1 lines:%2 Ln:%3 Col:%4")
            .arg(len).arg(lines).arg(curLine).arg(curCol)
    );
}

//显示/隐藏工具栏
void MainWindow::on_action_ShowToolBar_toggled(bool checked)
{
    ui->toolBar->setVisible(checked);
}

//显示/隐藏状态栏
void MainWindow::on_action_ShowStatusBar_toggled(bool checked)
{
    statusBar()->setVisible(checked);
}

// 定时自动保存：只对已经保存过且有改动的文件生效
void MainWindow::onAutoSaveTimeout()
{
    if (!m_isModified || m_curFilePath.isEmpty())
        return;

    QFile file(m_curFilePath + ".autosave");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.write(ui->plainTextEdit->toPlainText().toUtf8());
    file.close();
    statusBar()->showMessage(QString("已自动保存副本：%1.autosave").arg(QFileInfo(m_curFilePath).fileName()), 4000);
}
