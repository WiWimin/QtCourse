#include "doctormanagewidget.h"

#include "dbhelper.h"
#include "doctordialog.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QTableView>
#include <QVBoxLayout>

DoctorManageWidget::DoctorManageWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("Page");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 16, 20, 12);
    layout->setSpacing(12);

    QHBoxLayout *toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);
    QLabel *hint = new QLabel("医生所属科室从科室表中读取，保证外键合法", this);
    hint->setObjectName("HintLabel");

    QPushButton *btnAdd = new QPushButton("添加医生", this);
    btnAdd->setObjectName("PrimaryButton");
    QPushButton *btnEdit = new QPushButton("修改", this);
    QPushButton *btnDelete = new QPushButton("删除", this);
    btnDelete->setObjectName("DangerButton");
    QPushButton *btnRefresh = new QPushButton("刷新", this);

    foreach (QPushButton *btn, QList<QPushButton*>() << btnAdd << btnEdit << btnDelete << btnRefresh)
    {
        btn->setFixedHeight(34);
        btn->setMinimumWidth(90);
        btn->setCursor(Qt::PointingHandCursor);
    }

    toolbar->addWidget(hint);
    toolbar->addStretch(1);
    toolbar->addWidget(btnAdd);
    toolbar->addWidget(btnEdit);
    toolbar->addWidget(btnDelete);
    toolbar->addWidget(btnRefresh);
    layout->addLayout(toolbar);

    // QSqlQueryModel 使用 Qt 默认连接（DBHelper 把数据库注册在默认连接上），
    // 因此这里不需要（也没有）带数据库参数的构造函数。
    m_model = new QSqlQueryModel(this);

    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->verticalHeader()->setDefaultSectionSize(32);
    // 让最后一列（所属科室）自动占满右侧，表格不会留下大片空白
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setHighlightSections(false);
    layout->addWidget(m_tableView, 1);

    m_lblStatus = new QLabel(this);
    m_lblStatus->setObjectName("HintLabel");
    layout->addWidget(m_lblStatus);

    connect(btnAdd, &QPushButton::clicked, this, &DoctorManageWidget::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &DoctorManageWidget::onEdit);
    connect(btnDelete, &QPushButton::clicked, this, &DoctorManageWidget::onDelete);
    connect(btnRefresh, &QPushButton::clicked, this, &DoctorManageWidget::onRefresh);

    reload();
}

void DoctorManageWidget::reload()
{
    refreshTable();
}

void DoctorManageWidget::refreshTable()
{
    // 第 5 列 DEPARTMENT_ID 只用于修改时取外键，界面上隐藏
    m_model->setQuery(
        "SELECT d.ID, d.EMPLOYEEENO, d.NAME,"
        "       IFNULL(p.NAME, '(未分配)') AS DEPT_NAME,"
        "       d.DEPARTMENT_ID"
        " FROM Doctor d"
        " LEFT JOIN Department p ON d.DEPARTMENT_ID = p.ID"
        " ORDER BY d.ID");

    if (m_model->lastError().isValid())
    {
        QMessageBox::warning(this, "读取失败",
                             "读取医生数据失败：" + m_model->lastError().text());
        return;
    }

    m_model->setHeaderData(0, Qt::Horizontal, "医生编号");
    m_model->setHeaderData(1, Qt::Horizontal, "工号");
    m_model->setHeaderData(2, Qt::Horizontal, "姓名");
    m_model->setHeaderData(3, Qt::Horizontal, "所属科室");
    m_model->setHeaderData(4, Qt::Horizontal, "科室编号");

    // 没有 setStretchLastSection 时，列宽按设定值显示，右侧留白，避免“所属科室”被拉得过宽
    m_tableView->setColumnWidth(0, 110);
    m_tableView->setColumnWidth(1, 110);
    m_tableView->setColumnWidth(2, 110);
    m_tableView->setColumnWidth(3, 240);
    m_tableView->setColumnHidden(4, true);

    updateStatus();
}

void DoctorManageWidget::updateStatus()
{
    m_lblStatus->setText(QString("共 %1 位医生").arg(m_model->rowCount()));
}

QString DoctorManageWidget::selectedDoctorId() const
{
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid())
        return QString();
    return m_model->data(m_model->index(index.row(), 0)).toString();
}

void DoctorManageWidget::onAdd()
{
    if (DBHelper::getInstance()->departmentNames().isEmpty())
    {
        QMessageBox::information(this, "提示",
                                 "还没有科室，请先到“科室管理”中添加科室");
        return;
    }

    DoctorDialog dlg(this);
    dlg.setEditMode(false);
    dlg.setInfo(DBHelper::getInstance()->nextDoctorId(), QString(), QString(), QString());

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString message;
    if (!DBHelper::getInstance()->insertDoctor(dlg.doctorId(), dlg.employeeNo(),
                                               dlg.doctorName(), dlg.departmentId(),
                                               &message))
    {
        QMessageBox::warning(this, "新增失败", message);
        return;
    }

    DBHelper::getInstance()->logEvent(
                QString("新增医生 %1（%2）").arg(dlg.doctorName(), dlg.doctorId()));
    refreshTable();
}

void DoctorManageWidget::onEdit()
{
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid())
    {
        QMessageBox::information(this, "提示", "请先在表格中选中一位医生");
        return;
    }

    const QString id     = m_model->data(m_model->index(index.row(), 0)).toString();
    const QString empNo  = m_model->data(m_model->index(index.row(), 1)).toString();
    const QString name   = m_model->data(m_model->index(index.row(), 2)).toString();
    const QString deptId = m_model->data(m_model->index(index.row(), 4)).toString();

    DoctorDialog dlg(this);
    dlg.setEditMode(true);
    dlg.setInfo(id, empNo, name, deptId);
    if (dlg.exec() != QDialog::Accepted)
        return;

    QString message;
    if (!DBHelper::getInstance()->updateDoctor(id, dlg.employeeNo(),
                                               dlg.doctorName(), dlg.departmentId(),
                                               &message))
    {
        QMessageBox::warning(this, "修改失败", message);
        return;
    }

    DBHelper::getInstance()->logEvent(QString("修改医生 %1（%2）").arg(dlg.doctorName(), id));
    refreshTable();
}

void DoctorManageWidget::onDelete()
{
    const QString id = selectedDoctorId();
    if (id.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先在表格中选中要删除的医生");
        return;
    }

    const QModelIndex index = m_tableView->currentIndex();
    const QString name = m_model->data(m_model->index(index.row(), 2)).toString();

    if (QMessageBox::question(this, "删除确认",
                              QString("确定要删除医生【%1】（编号 %2）吗？").arg(name, id),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

    QString message;
    if (!DBHelper::getInstance()->deleteDoctor(id, &message))
    {
        QMessageBox::warning(this, "删除失败", message);
        return;
    }

    DBHelper::getInstance()->logEvent(QString("删除医生 %1（%2）").arg(name, id));
    refreshTable();
}

void DoctorManageWidget::onRefresh()
{
    refreshTable();
}
