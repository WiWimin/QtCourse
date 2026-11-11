#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

class QLabel;
class QTimer;
class FindDialog;
class ReplaceDialog;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 重写窗口关闭事件（重点）
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 菜单Action槽
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();
    void on_action_SaveAs_triggered();
    void on_action_Quit_triggered();
    void on_action_About_triggered();

    // 文本变化槽，监听修改
    void on_plainTextEdit_textChanged();

    void on_plainTextEdit_copyAvailable(bool flag);
    // 撤销、重做可用性信号槽
    void onUndoAvailable(bool canUndoFlag);
    void onRedoAvailable(bool canRedoFlag);

    //格式菜单槽
    void on_action_Wrap_toggled(bool checked);
    void on_action_ShowLine_toggled(bool checked);
    void on_action_Font_triggered();
    void on_action_FontColor_triggered();
    void on_action_SelBgColor_triggered();
    void on_action_EditBgColor_triggered();

    // 查找、替换菜单槽
    void on_action_Find_triggered();
    void on_action_Replace_triggered();
    // 来自查找/替换对话框的请求
    void onFindNextRequested();
    void onReplaceRequested();
    void onReplaceAllRequested();

    // 状态栏更新槽
    void updateStatusBar();

    void on_action_ShowToolBar_toggled(bool checked);
    void on_action_ShowStatusBar_toggled(bool checked);

    // 定时自动保存（选做）
    void onAutoSaveTimeout();

private:
    Ui::MainWindow *ui;
    // ========= 新增成员变量 =========
    QString m_curFilePath;
    bool m_isModified;

    FindDialog *m_findDialog;
    ReplaceDialog *m_replaceDialog;
    QLabel *m_statusLabel;      // 状态栏右侧常驻信息（长度/行数/行列号）
    QTimer *m_autoSaveTimer;    // 定时自动保存

    // ========= 自定义函数声明 =========
    void updateWindowTitle();
    bool maybeSave();
    bool loadFile(const QString &filePath);
    bool saveFile(const QString &filePath);

    void doFindNext(const QString &text, bool caseSensitive, bool backward);
    int doReplaceAll(const QString &findText, const QString &replaceText, bool caseSensitive);
};
#endif // MAINWINDOW_H
