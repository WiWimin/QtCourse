#ifndef PATIENTMANAGEWIDGET_H
#define PATIENTMANAGEWIDGET_H

#include <QWidget>
#include <QAbstractItemDelegate>

class QTableView;
class PatientTableModel;
class QLineEdit;
class QLabel;
class QPushButton;

/*
 * 患者管理页面（View 层）
 * ------------------------------------------------------------------
 * 核心：QTableView + QSqlTableModel（Model/View 结构）+ 自定义代理。
 * 页面只负责：
 *   - 收集查询条件、显示表格；
 *   - 调用 DBHelper 完成增删改；
 *   - 操作后 refreshTable()，让视图与数据库保持一致。
 * 页面里不出现任何 SQL 语句。
 */
class PatientManageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PatientManageWidget(QWidget *parent = nullptr);

    // 进入页面时调用：清空查询条件并重新读取全部数据
    void reloadData();

signals:
    void goBackHome();   // 页面左上角“返回主页”

private slots:
    void onSearch();
    void onAdd();
    void onEdit();
    void onDelete();
    void onRefreshClicked();
    // 表格内通过代理编辑结束后，把修改提交到数据库
    void onEditorClosed(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

private:
    void refreshTable();         // 保留当前查询条件刷新
    void applyHeaderTitles();    // 设置中文表头
    void applyColumnWidths();
    void updateStatus();
    QString selectedPatientId() const;
    QString selectedPatientName() const;

    QLineEdit       *m_editSearch;
    QPushButton     *m_btnSearch;
    QTableView      *m_tableView;
    PatientTableModel *m_model;
    QLabel          *m_lblStatus;
    QString          m_currentKeyword;   // 当前生效的查询关键字
};

#endif // PATIENTMANAGEWIDGET_H
