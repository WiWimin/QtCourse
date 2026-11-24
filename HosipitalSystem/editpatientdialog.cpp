#include "editpatientdialog.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

EditPatientDialog::EditPatientDialog(QWidget *parent)
    : QDialog(parent)
    , m_mode(AddMode)
{
    setWindowTitle("新增患者信息");
    setFixedWidth(460);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(26, 20, 26, 18);
    layout->setSpacing(12);

    // ---------------- 表单 ----------------
    QFormLayout *form = new QFormLayout();
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_editId = new QLineEdit(this);
    m_editId->setPlaceholderText("例如：P004（主键，不能重复）");
    m_editId->setMaxLength(20);

    m_editName = new QLineEdit(this);
    m_editName->setPlaceholderText("必填，患者姓名");
    m_editName->setMaxLength(30);

    m_editIdCard = new QLineEdit(this);
    m_editIdCard->setPlaceholderText("必填，15 或 18 位身份证号");
    m_editIdCard->setMaxLength(18);

    m_comboSex = new QComboBox(this);
    m_comboSex->addItem("男");
    m_comboSex->addItem("女");

    m_dateDob = new QDateEdit(this);
    m_dateDob->setCalendarPopup(true);
    m_dateDob->setDisplayFormat("yyyy-MM-dd");
    m_dateDob->setDate(QDate(2000, 1, 1));
    m_dateDob->setMaximumDate(QDate::currentDate());   // 出生日期不能是未来

    m_spinHeight = new QDoubleSpinBox(this);
    m_spinHeight->setRange(0, 300);
    m_spinHeight->setDecimals(1);
    m_spinHeight->setSuffix(" cm");
    m_spinHeight->setValue(170);

    m_spinWeight = new QDoubleSpinBox(this);
    m_spinWeight->setRange(0, 500);
    m_spinWeight->setDecimals(1);
    m_spinWeight->setSuffix(" kg");
    m_spinWeight->setValue(60);

    m_editMobile = new QLineEdit(this);
    m_editMobile->setPlaceholderText("必填，11 位手机号");
    m_editMobile->setMaxLength(11);

    form->addRow("ID：", m_editId);
    form->addRow("姓名：", m_editName);
    form->addRow("身份证：", m_editIdCard);
    form->addRow("性别：", m_comboSex);
    form->addRow("出生日期：", m_dateDob);
    form->addRow("身高：", m_spinHeight);
    form->addRow("体重：", m_spinWeight);
    form->addRow("手机号：", m_editMobile);

    layout->addLayout(form);

    // ---------------- 年龄提示（由出生日期自动推算） ----------------
    m_lblAgeHint = new QLabel(this);
    m_lblAgeHint->setObjectName("HintLabel");
    layout->addWidget(m_lblAgeHint);

    // ---------------- 错误提示 ----------------
    m_lblError = new QLabel(this);
    m_lblError->setObjectName("ErrorLabel");
    m_lblError->setMinimumHeight(20);
    m_lblError->setWordWrap(true);
    layout->addWidget(m_lblError);

    // ---------------- 按钮 ----------------
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch(1);
    QPushButton *btnCancel = new QPushButton("取消", this);
    QPushButton *btnSave = new QPushButton("保存", this);
    btnSave->setObjectName("PrimaryButton");
    btnCancel->setFixedSize(100, 34);
    btnSave->setFixedSize(100, 34);
    btnSave->setDefault(true);
    btnRow->addWidget(btnCancel);
    btnRow->addWidget(btnSave);
    layout->addLayout(btnRow);

    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, this, &EditPatientDialog::onSaveClicked);
    connect(m_dateDob, &QDateEdit::dateChanged, this, &EditPatientDialog::onDobChanged);

    onDobChanged();
    m_editId->setFocus();
}

void EditPatientDialog::setMode(Mode mode)
{
    m_mode = mode;
    if (mode == EditMode)
    {
        setWindowTitle("修改患者信息");
        m_editId->setReadOnly(true);      // 主键不允许修改
        m_editId->setToolTip("患者编号是主键，修改时不可更改");
        m_editName->setFocus();
    }
    else
    {
        setWindowTitle("新增患者信息");
        m_editId->setReadOnly(false);
    }
}

void EditPatientDialog::setPatientInfo(const PatientInfo& info)
{
    m_info = info;

    m_editId->setText(info.id);
    m_editName->setText(info.name);
    m_editIdCard->setText(info.idCard);
    m_comboSex->setCurrentIndex(info.sex == 0 ? 0 : 1);

    QDate dob = QDate::fromString(info.dob, "yyyy-MM-dd");
    if (!dob.isValid())
        dob = QDate(2000, 1, 1);
    m_dateDob->setDate(dob);

    m_spinHeight->setValue(info.height);
    m_spinWeight->setValue(info.weight);
    m_editMobile->setText(info.mobile);

    onDobChanged();
}

PatientInfo EditPatientDialog::patientInfo() const
{
    return m_info;
}

void EditPatientDialog::onDobChanged()
{
    const int age = ageFromDate(m_dateDob->date());
    m_lblAgeHint->setText(QString("按出生日期自动计算年龄：%1 岁").arg(age));
}

/*
 * 保存：先做界面校验，全部通过才把控件内容收进 m_info 并 accept()
 */
void EditPatientDialog::onSaveClicked()
{
    QString errorText;
    QWidget *errorWidget = nullptr;

    if (!validateInput(&errorText, &errorWidget))
    {
        m_lblError->setText(errorText);
        if (errorWidget)
            errorWidget->setFocus();
        return;
    }
    m_lblError->clear();

    PatientInfo info;
    info.id               = m_editId->text().trimmed();
    info.name             = m_editName->text().trimmed();
    info.idCard           = m_editIdCard->text().trimmed().toUpper();
    info.sex              = m_comboSex->currentIndex();
    info.dob              = m_dateDob->date().toString("yyyy-MM-dd");
    info.height           = m_spinHeight->value();
    info.weight           = m_spinWeight->value();
    info.mobile           = m_editMobile->text().trimmed();
    info.age              = ageFromDate(m_dateDob->date());
    // 修改时保留原建档时间；新增时写入当前时间
    info.createdTimestamp = m_info.createdTimestamp;
    if (info.createdTimestamp.isEmpty())
    {
        info.createdTimestamp = QDateTime::currentDateTime()
                                    .toString("yyyy-MM-dd HH:mm:ss");
    }

    m_info = info;
    accept();
}

/*
 * 必填项校验
 * -------------------------------------------------------------
 * 1. ID 非空（新增时）；2. 姓名非空；3. 身份证格式；
 * 4. 手机号格式；5. 身高体重必须大于 0。
 */
bool EditPatientDialog::validateInput(QString *errorText, QWidget **errorWidget)
{
    const QString id     = m_editId->text().trimmed();
    const QString name   = m_editName->text().trimmed();
    const QString idCard = m_editIdCard->text().trimmed();
    const QString mobile = m_editMobile->text().trimmed();

    if (id.isEmpty())
    {
        *errorText = "患者 ID 不能为空";
        *errorWidget = m_editId;
        return false;
    }
    if (name.isEmpty())
    {
        *errorText = "患者姓名不能为空";
        *errorWidget = m_editName;
        return false;
    }
    if (idCard.isEmpty())
    {
        *errorText = "身份证号不能为空";
        *errorWidget = m_editIdCard;
        return false;
    }

    // 15 位纯数字，或 18 位（末位可为 X）
    QRegularExpression idRe("^(\\d{15}|\\d{17}[\\dXx])$");
    if (!idRe.match(idCard).hasMatch())
    {
        *errorText = "身份证号格式不正确（应为 15 位或 18 位）";
        *errorWidget = m_editIdCard;
        return false;
    }

    if (mobile.isEmpty())
    {
        *errorText = "手机号不能为空";
        *errorWidget = m_editMobile;
        return false;
    }
    QRegularExpression mobileRe("^1\\d{10}$");
    if (!mobileRe.match(mobile).hasMatch())
    {
        *errorText = "手机号格式不正确（应为 11 位，以 1 开头）";
        *errorWidget = m_editMobile;
        return false;
    }

    if (m_spinHeight->value() <= 0)
    {
        *errorText = "身高必须大于 0";
        *errorWidget = m_spinHeight;
        return false;
    }
    if (m_spinWeight->value() <= 0)
    {
        *errorText = "体重必须大于 0";
        *errorWidget = m_spinWeight;
        return false;
    }

    return true;
}

int EditPatientDialog::ageFromDate(const QDate &birth) const
{
    if (!birth.isValid())
        return 0;
    const QDate today = QDate::currentDate();
    int age = today.year() - birth.year();
    if (today.month() < birth.month() ||
        (today.month() == birth.month() && today.day() < birth.day()))
    {
        --age;
    }
    return age < 0 ? 0 : age;
}
