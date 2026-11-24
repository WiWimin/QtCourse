#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;

/*
 * 注册对话框：把新账号写入 User 表
 */
class RegisterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RegisterDialog(QWidget *parent = nullptr);

    QString username() const;
    QString password() const;

private slots:
    void onSaveClicked();

private:
    QLineEdit *m_editUser;
    QLineEdit *m_editFullName;
    QLineEdit *m_editPwd;
    QLineEdit *m_editPwd2;
    QLabel    *m_lblTip;
};

#endif // REGISTERDIALOG_H
