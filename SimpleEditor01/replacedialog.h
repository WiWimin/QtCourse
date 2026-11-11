#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

namespace Ui {
class ReplaceDialog;
}

/**
 * 替换对话框（非模态）：收集“查找目标/替换为/区分大小写/方向”，
 * 具体替换动作由 MainWindow 执行。
 */
class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog();

    QString findText() const;
    QString replaceText() const;
    bool isCaseSensitive() const;
    bool isBackward() const;

signals:
    void findNextRequested();
    void replaceRequested();
    void replaceAllRequested();

private slots:
    void on_btnFindNext_clicked();
    void on_btnReplace_clicked();
    void on_btnReplaceAll_clicked();

private:
    Ui::ReplaceDialog *ui;
};

#endif // REPLACEDIALOG_H
