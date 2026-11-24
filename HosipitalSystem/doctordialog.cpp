#include "doctordialog.h"

#include "dbhelper.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

DoctorDialog::DoctorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("医生信息");
    setFixedWidth(420);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 18);
    layout->setSpacing(12);

    QFormLayout *form = new QFormLayout();
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    m_editId = new QLineEdit(this);
    m_editId->setReadOnly(true);          // 编号自动生成，不允许手改

    m_editNo = new QLineEdit(this);
    m_editNo->setPlaceholderText("例如：E1003");

    m_editName = new QLineEdit(this);
    m_editName->setPlaceholderText("必填，医生姓名");

    m_comboDept = new QComboBox(this);
    // 下拉项显示科室名称，userData 保存科室编号（外键值）
    const QStringList names = DBHelper::getInstance()->departmentNames();
    foreach (const QString& name, names)
    {
        m_comboDept->addItem(name,
                             DBHelper::getInstance()->departmentIdByName(name));
    }

    form->addRow("医生编号：", m_editId);
    form->addRow("工号：", m_editNo);
    form->addRow("姓名：", m_editName);
    form->addRow("所属科室：", m_comboDept);
    layout->addLayout(form);

    m_lblError = new QLabel(this);
    m_lblError->setObjectName("ErrorLabel");
    m_lblError->setMinimumHeight(20);
    layout->addWidget(m_lblError);

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    QPushButton *btnCancel = new QPushButton("取消", this);
    QPushButton *btnSave = new QPushButton("保存", this);
    btnSave->setObjectName("PrimaryButton");
    btnCancel->setFixedSize(100, 34);
    btnSave->setFixedSize(100, 34);
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSave);
    layout->addLayout(btnRow);

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, this, &DoctorDialog::onSaveClicked);

    m_editName->setFocus();
}

void DoctorDialog::setInfo(const QString& id,
                           const QString& employeeNo,
                           const QString& name,
                           const QString& departmentId)
{
    m_editId->setText(id);
    m_editNo->setText(employeeNo);
    m_editName->setText(name);

    const int index = m_comboDept->findData(departmentId);
    if (index >= 0)
        m_comboDept->setCurrentIndex(index);
}

void DoctorDialog::setEditMode(bool edit)
{
    setWindowTitle(edit ? "修改医生信息" : "新增医生");
}

QString DoctorDialog::doctorId() const       { return m_editId->text().trimmed(); }
QString DoctorDialog::employeeNo() const     { return m_editNo->text().trimmed(); }
QString DoctorDialog::doctorName() const     { return m_editName->text().trimmed(); }
QString DoctorDialog::departmentId() const   { return m_comboDept->currentData().toString(); }

void DoctorDialog::onSaveClicked()
{
    if (m_editId->text().trimmed().isEmpty())
    {
        m_lblError->setText("医生编号为空，请检查数据库初始化");
        return;
    }
    if (doctorName().isEmpty())
    {
        m_lblError->setText("医生姓名不能为空");
        m_editName->setFocus();
        return;
    }
    if (m_comboDept->count() == 0)
    {
        m_lblError->setText("还没有科室，请先到“科室管理”中添加科室");
        return;
    }

    m_lblError->clear();
    accept();
}
