#ifndef DEPTMANAGEWIDGET_H
#define DEPTMANAGEWIDGET_H

#include <QWidget>

class QTableView;
class QSqlTableModel;
class QLabel;

/*
 * 科室管理页面（选做功能）
 * 用 QSqlTableModel 直接映射 Department 表，支持增、改、删。
 */
class DeptManageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeptManageWidget(QWidget *parent = nullptr);

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
    QString selectedDepartmentId() const;
    QString selectedDepartmentName() const;

    QSqlTableModel *m_model;
    QTableView     *m_tableView;
    QLabel         *m_lblStatus;
};

#endif // DEPTMANAGEWIDGET_H
