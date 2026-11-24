#include "loginwidget.h"
#include "registerdialog.h"
#include "dbhelper.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFont>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("Page");

    // 外层：上下弹簧 + 居中卡片
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->addStretch(1);

    QFrame *card = new QFrame(this);
    card->setObjectName("Card");
    card->setFixedWidth(470);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(36, 28, 36, 32);
    cardLayout->setSpacing(10);

    // ---------- 标题 ----------
    QLabel *title = new QLabel("欢迎使用诊疗测试系统", card);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);

    QLabel *subtitle = new QLabel("医院诊疗患者信息管理系统", card);
    subtitle->setObjectName("HintLabel");
    subtitle->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(title);
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(18);

    // ---------- 表单 ----------
    QGridLayout *form = new QGridLayout();
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(14);

    QLabel *lblUser = new QLabel("用户名：", card);
    lblUser->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_editUser = new QLineEdit(card);
    m_editUser->setPlaceholderText("请输入用户名");
    m_editUser->setText("admin");          // 方便课堂演示
    m_editUser->setFixedHeight(34);

    QLabel *lblPwd = new QLabel("密码：", card);
    lblPwd->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_editPwd = new QLineEdit(card);
    m_editPwd->setPlaceholderText("请输入密码");
    m_editPwd->setText("123456");          // 方便课堂演示
    m_editPwd->setEchoMode(QLineEdit::Password);
    m_editPwd->setFixedHeight(34);

    form->addWidget(lblUser, 0, 0);
    form->addWidget(m_editUser, 0, 1);
    form->addWidget(lblPwd, 1, 0);
    form->addWidget(m_editPwd, 1, 1);
    form->setColumnMinimumWidth(0, 70);
    form->setColumnStretch(1, 1);
    cardLayout->addLayout(form);

    // ---------- 错误提示（区分：用户名不存在 / 密码错误 / 数据库未连接） ----------
    m_lblError = new QLabel("", card);
    m_lblError->setObjectName("ErrorLabel");
    m_lblError->setAlignment(Qt::AlignCenter);
    m_lblError->setMinimumHeight(20);
    cardLayout->addWidget(m_lblError);

    // ---------- 按钮 ----------
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);

    QPushButton *btnReg = new QPushButton("注册", card);
    btnReg->setFixedSize(120, 36);

    m_btnLogin = new QPushButton("登录", card);
    m_btnLogin->setObjectName("PrimaryButton");
    m_btnLogin->setFixedSize(120, 36);
    m_btnLogin->setDefault(true);          // 回车时触发默认按钮

    btnRow->addWidget(btnReg);
    btnRow->addWidget(m_btnLogin);
    btnRow->addStretch(1);
    cardLayout->addSpacing(4);
    cardLayout->addLayout(btnRow);

    outer->addWidget(card, 0, Qt::AlignHCenter);
    outer->addStretch(1);
    setLayout(outer);

    // ---------- 信号槽 ----------
    connect(m_btnLogin, &QPushButton::clicked, this, &LoginWidget::onLoginClicked);
    connect(btnReg, &QPushButton::clicked, this, &LoginWidget::onRegisterClicked);
    // 在密码框按回车 = 点登录
    connect(m_editPwd, &QLineEdit::returnPressed, this, &LoginWidget::onLoginClicked);
}

void LoginWidget::resetInput()
{
    m_lblError->clear();
    m_editUser->setFocus();
    m_editUser->selectAll();
}

/*
 * 登录：调用 DBHelper::checkLogin()
 * -------------------------------------------------------------
 * 校验顺序：连接是否可用 → 用户名是否存在 → 密码是否匹配，
 * 因此可以给出比“用户名或密码错误”更精确的提示。
 */
void LoginWidget::onLoginClicked()
{
    m_lblError->clear();

    const QString username = m_editUser->text().trimmed();
    const QString password = m_editPwd->text().trimmed();

    // 1. 界面层的基础校验：空输入不发 SQL
    if (username.isEmpty())
    {
        showError("请输入用户名");
        m_editUser->setFocus();
        return;
    }
    if (password.isEmpty())
    {
        showError("请输入密码");
        m_editPwd->setFocus();
        return;
    }

    // 2. 交给 Model 层校验
    QString userId;
    QString fullName;
    QString message;
    if (!DBHelper::getInstance()->checkLogin(username, password, &userId, &fullName, &message))
    {
        showError(message);
        m_editPwd->selectAll();
        m_editPwd->setFocus();
        return;
    }

    // 3. 记录当前用户 + 写操作日志
    DBHelper::getInstance()->setActiveUser(userId, fullName);
    DBHelper::getInstance()->logEvent(QString("登录系统（用户名：%1）").arg(username));

    emit loginSuccess(userId, fullName);
}

void LoginWidget::onRegisterClicked()
{
    RegisterDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        m_lblError->setText(QString("账号 %1 注册成功，请登录").arg(dlg.username()));
        // 动态样式优先级高于全局 QSS，用来把错误红字变成成功绿字
        m_lblError->setStyleSheet("color:#059669;");
        m_editUser->setText(dlg.username());
        m_editPwd->setText(dlg.password());
    }
}

void LoginWidget::showError(const QString& text)
{
    m_lblError->setStyleSheet(QString());   // 恢复全局 QSS 的红色
    m_lblError->setText(text);
}
