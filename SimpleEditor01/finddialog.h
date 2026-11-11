#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

/**
 * 查找对话框（非模态）：只负责收集查找条件，
 * 真正的查找动作由 MainWindow 完成，通过 findNextRequested() 信号通知。
 */
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog();

    QString findText() const;
    bool isCaseSensitive() const;
    bool isBackward() const;      // true = 向上查找

    void setFindText(const QString &text);

signals:
    void findNextRequested();

private slots:
    void on_btnFindNext_clicked();

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
