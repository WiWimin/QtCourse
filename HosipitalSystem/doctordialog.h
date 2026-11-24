#ifndef DOCTORDIALOG_H
#define DOCTORDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QLabel;

/*
 * 医生信息编辑对话框（选做功能）
 * 编号由程序自动生成；科室用下拉框选择，保证外键合法。
 */
class DoctorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DoctorDialog(QWidget *parent = nullptr);

    void setInfo(const QString& id,
                 const QString& employeeNo,
                 const QString& name,
                 const QString& departmentId);
    void setEditMode(bool edit);

    QString doctorId() const;
    QString employeeNo() const;
    QString doctorName() const;
    QString departmentId() const;

private slots:
    void onSaveClicked();

private:
    QLineEdit *m_editId;
    QLineEdit *m_editNo;
    QLineEdit *m_editName;
    QComboBox *m_comboDept;
    QLabel    *m_lblError;
};

#endif // DOCTORDIALOG_H
