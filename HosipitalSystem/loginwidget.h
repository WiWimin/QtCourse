#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;

/*
 * 登录页面（View 层）
 * ------------------------------------------------------------------
 * 只负责：收集用户名/密码、展示错误提示、发出登录成功信号。
 * 真正的账号校验在 DBHelper::checkLogin() 中完成。
 */
class LoginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWidget(QWidget *parent = nullptr);

    // 退出登录后回到登录页时，清空密码并聚焦用户名
    void resetInput();

signals:
    // 登录成功后把当前用户信息交给主窗口
    void loginSuccess(const QString& userId, const QString& fullName);

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void showError(const QString& text);

    QLineEdit   *m_editUser;
    QLineEdit   *m_editPwd;
    QLabel      *m_lblError;
    QPushButton *m_btnLogin;
};

#endif // LOGINWIDGET_H
