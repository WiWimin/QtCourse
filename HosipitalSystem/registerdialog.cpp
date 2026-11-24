#include "registerdialog.h"
#include "dbhelper.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("注册新账号");
    setFixedWidth(420);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(12);

    QLabel *title = new QLabel("新建登录账号", this);
    QFont f = title->font();
    f.setPointSize(13);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_editUser = new QLineEdit(this);
    m_editUser->setPlaceholderText("3 位以上，不能重复");
    m_editFullName = new QLineEdit(this);
    m_editFullName->setPlaceholderText("例如：门诊医生");
    m_editPwd = new QLineEdit(this);
    m_editPwd->setEchoMode(QLineEdit::Password);
    m_editPwd->setPlaceholderText("至少 6 位");
    m_editPwd2 = new QLineEdit(this);
    m_editPwd2->setEchoMode(QLineEdit::Password);
    m_editPwd2->setPlaceholderText("再次输入密码");

    form->addRow("用户名：", m_editUser);
    form->addRow("姓名：", m_editFullName);
    form->addRow("密码：", m_editPwd);
    form->addRow("确认密码：", m_editPwd2);
    layout->addLayout(form);

    m_lblTip = new QLabel("", this);
    m_lblTip->setObjectName("ErrorLabel");
    m_lblTip->setMinimumHeight(20);
    layout->addWidget(m_lblTip);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    QPushButton *btnCancel = new QPushButton("取消", this);
    QPushButton *btnSave = new QPushButton("注册", this);
    btnSave->setObjectName("PrimaryButton");
    btnCancel->setFixedSize(100, 34);
    btnSave->setFixedSize(100, 34);
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSave);
    layout->addLayout(btnRow);

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, this, &RegisterDialog::onSaveClicked);

    m_editUser->setFocus();
}

QString RegisterDialog::username() const
{
    return m_editUser->text().trimmed();
}

QString RegisterDialog::password() const
{
    return m_editPwd->text();
}

void RegisterDialog::onSaveClicked()
{
    // 界面层校验：空值、长度、两次密码一致性
    const QString user = m_editUser->text().trimmed();
    const QString name = m_editFullName->text().trimmed();
    const QString pwd  = m_editPwd->text();
    const QString pwd2 = m_editPwd2->text();

    if (user.isEmpty() || pwd.isEmpty())
    {
        m_lblTip->setText("用户名和密码不能为空");
        return;
    }
    if (user.length() < 3)
    {
        m_lblTip->setText("用户名至少 3 位");
        return;
    }
    if (pwd.length() < 6)
    {
        m_lblTip->setText("密码至少 6 位");
        return;
    }
    if (pwd != pwd2)
    {
        m_lblTip->setText("两次输入的密码不一致");
        return;
    }

    QString message;
    if (!DBHelper::getInstance()->registerUser(user, pwd,
                                               name.isEmpty() ? user : name,
                                               &message))
    {
        m_lblTip->setText(message);
        return;
    }

    accept();
}
