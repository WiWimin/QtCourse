#include "deptmanagewidget.h"

#include "dbhelper.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>

DeptManageWidget::DeptManageWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("Page");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 16, 20, 12);
    layout->setSpacing(12);

    QHBoxLayout *toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);
    QLabel *hint = new QLabel("科室是医生所属部门，被医生引用的科室不能删除", this);
    hint->setObjectName("HintLabel");

    QPushButton *btnAdd = new QPushButton("新增科室", this);
    btnAdd->setObjectName("PrimaryButton");
    QPushButton *btnEdit = new QPushButton("修改名称", this);
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

    m_model = new QSqlTableModel(this, DBHelper::getInstance()->database());
    m_model->setTable("Department");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 统一走按钮编辑
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->verticalHeader()->setDefaultSectionSize(32);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setHighlightSections(false);
    layout->addWidget(m_tableView, 1);

    m_lblStatus = new QLabel(this);
    m_lblStatus->setObjectName("HintLabel");
    layout->addWidget(m_lblStatus);

    connect(btnAdd, &QPushButton::clicked, this, &DeptManageWidget::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &DeptManageWidget::onEdit);
    connect(btnDelete, &QPushButton::clicked, this, &DeptManageWidget::onDelete);
    connect(btnRefresh, &QPushButton::clicked, this, &DeptManageWidget::onRefresh);

    reload();
}

void DeptManageWidget::reload()
{
    refreshTable();
}

void DeptManageWidget::refreshTable()
{
    const bool selectOk = m_model->select();
    if (!selectOk)
    {
        QMessageBox::warning(this, "读取失败",
                             "读取科室数据失败：" + m_model->lastError().text());
        return;
    }
    m_model->setHeaderData(0, Qt::Horizontal, "科室编号");
    m_model->setHeaderData(1, Qt::Horizontal, "科室名称");
    m_tableView->setColumnWidth(0, 120);
    m_tableView->setColumnWidth(1, 320);
    updateStatus();
}

void DeptManageWidget::updateStatus()
{
    m_lblStatus->setText(QString("共 %1 个科室").arg(m_model->rowCount()));
}

QString DeptManageWidget::selectedDepartmentId() const
{
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid())
        return QString();
    return m_model->data(m_model->index(index.row(), 0)).toString();
}

QString DeptManageWidget::selectedDepartmentName() const
{
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid())
        return QString();
    return m_model->data(m_model->index(index.row(), 1)).toString();
}

void DeptManageWidget::onAdd()
{
    bool ok = false;
    const QString name = QInputDialog::getText(
                this, "新增科室", "请输入科室名称：",
                QLineEdit::Normal, QString(), &ok).trimmed();
    if (!ok)
        return;

    const QString id = DBHelper::getInstance()->nextDepartmentId();
    QString message;
    if (!DBHelper::getInstance()->insertDepartment(id, name, &message))
    {
        QMessageBox::warning(this, "新增失败", message);
        return;
    }

    DBHelper::getInstance()->logEvent(QString("新增科室 %1（%2）").arg(name, id));
    refreshTable();
    QMessageBox::information(this, "新增成功",
                             QString("科室“%1”已添加，编号 %2").arg(name, id));
}

void DeptManageWidget::onEdit()
{
    const QString id = selectedDepartmentId();
    if (id.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先在表格中选中一个科室");
        return;
    }

    bool ok = false;
    const QString name = QInputDialog::getText(
                this, "修改科室名称", "科室名称：",
                QLineEdit::Normal, selectedDepartmentName(), &ok).trimmed();
    if (!ok)
        return;

    QString message;
    if (!DBHelper::getInstance()->updateDepartment(id, name, &message))
    {
        QMessageBox::warning(this, "修改失败", message);
        return;
    }

    DBHelper::getInstance()->logEvent(QString("修改科室 %1 名称为 %2").arg(id, name));
    refreshTable();
}

void DeptManageWidget::onDelete()
{
    const QString id = selectedDepartmentId();
    if (id.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先在表格中选中要删除的科室");
        return;
    }

    if (QMessageBox::question(this, "删除确认",
                              QString("确定删除科室【%1】吗？").arg(selectedDepartmentName()),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

    QString message;
    if (!DBHelper::getInstance()->deleteDepartment(id, &message))
    {
        QMessageBox::warning(this, "删除失败", message);
        return;
    }

    DBHelper::getInstance()->logEvent(QString("删除科室 %1").arg(id));
    refreshTable();
}

void DeptManageWidget::onRefresh()
{
    refreshTable();
}
