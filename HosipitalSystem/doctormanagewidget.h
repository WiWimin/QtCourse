#ifndef DOCTORMANAGEWIDGET_H
#define DOCTORMANAGEWIDGET_H

#include <QWidget>

class QTableView;
class QSqlQueryModel;
class QLabel;

/*
 * 医生管理页面（选做功能）
 * 医生表通过 DEPARTMENT_ID 关联科室，因此用带 JOIN 的 QSqlQueryModel
 * 显示“科室名称”，而新增/修改由 DoctorDialog 完成。
 */
class DoctorManageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DoctorManageWidget(QWidget *parent = nullptr);

    void reload();

signals:
    void goBackHome();

private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onRefresh();

private:
    void refreshTable();
    void updateStatus();
    QString selectedDoctorId() const;

    QSqlQueryModel *m_model;
    QTableView     *m_tableView;
    QLabel         *m_lblStatus;
};

#endif // DOCTORMANAGEWIDGET_H
