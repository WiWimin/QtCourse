#ifndef EDITPATIENTDIALOG_H
#define EDITPATIENTDIALOG_H

#include <QDialog>

#include "dbhelper.h"   // PatientInfo 结构体

class QLineEdit;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLabel;

/*
 * 新增 / 修改患者信息对话框
 * ------------------------------------------------------------------
 * 同一个对话框支持两种模式：
 *   AddMode  —— 新增，ID 可填写；
 *   EditMode —— 修改，ID 只读，避免改主键。
 * 保存前在界面层做必填校验，真正写库由 DBHelper 负责。
 */
class EditPatientDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode { AddMode, EditMode };

    explicit EditPatientDialog(QWidget *parent = nullptr);

    void setMode(Mode mode);
    void setPatientInfo(const PatientInfo& info);

    // 校验通过并点“保存”后，外部通过该函数取回数据
    PatientInfo patientInfo() const;

private slots:
    void onSaveClicked();
    void onDobChanged();

private:
    bool validateInput(QString *errorText, QWidget **errorWidget);
    int  ageFromDate(const QDate& birth) const;

    Mode m_mode;

    QLineEdit      *m_editId;
    QLineEdit      *m_editName;
    QLineEdit      *m_editIdCard;
    QComboBox      *m_comboSex;
    QDateEdit      *m_dateDob;
    QDoubleSpinBox *m_spinHeight;
    QDoubleSpinBox *m_spinWeight;
    QLineEdit      *m_editMobile;
    QLabel         *m_lblAgeHint;
    QLabel         *m_lblError;

    PatientInfo     m_info;     // 保存成功前的数据暂存
};

#endif // EDITPATIENTDIALOG_H
