#include "patientmanagewidget.h"

#include "dbhelper.h"
#include "editpatientdialog.h"
#include "patientdelegate.h"
#include "patienttablemodel.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlTableModel>
#include <QTableView>
#include <QVBoxLayout>

PatientManageWidget::PatientManageWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("Page");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 16, 20, 12);
    layout->setSpacing(12);

    // ================= 顶部操作栏（代码化创建控件） =================
    QHBoxLayout *toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);

    m_editSearch = new QLineEdit(this);
    m_editSearch->setPlaceholderText("按姓名 / 身份证 / 手机号查询");
    m_editSearch->setClearButtonEnabled(true);
    m_editSearch->setFixedHeight(34);
    m_editSearch->setMinimumWidth(240);

    m_btnSearch = new QPushButton("查找", this);
    m_btnSearch->setObjectName("PrimaryButton");
    m_btnSearch->setFixedHeight(34);
    m_btnSearch->setMinimumWidth(76);

    QPushButton *btnAdd = new QPushButton("添加", this);
    QPushButton *btnEdit = new QPushButton("修改", this);
    QPushButton *btnDelete = new QPushButton("删除", this);
    btnDelete->setObjectName("DangerButton");
    QPushButton *btnRefresh = new QPushButton("刷新", this);

    QList<QPushButton*> actionButtons = QList<QPushButton*>()
                                        << m_btnSearch << btnAdd << btnEdit
                                        << btnDelete << btnRefresh;
    foreach (QPushButton *btn, actionButtons)
    {
        btn->setFixedHeight(34);
        btn->setMinimumWidth(76);
        btn->setCursor(Qt::PointingHandCursor);
    }

    toolbar->addWidget(m_editSearch, 1);
    toolbar->addWidget(m_btnSearch);
    toolbar->addSpacing(8);
    toolbar->addWidget(btnAdd);
    toolbar->addWidget(btnEdit);
    toolbar->addWidget(btnDelete);
    toolbar->addWidget(btnRefresh);

    layout->addLayout(toolbar);

    // ================= 表格：QTableView + QSqlTableModel =================
    m_model = new PatientTableModel(this, DBHelper::getInstance()->database());
    m_model->setTable("Patient");
    // 手动提交：代理编辑完调用 submitAll()，出错可以回滚
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    m_tableView = new QTableView(this);
    m_tableView->setModel(m_model);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setSortingEnabled(false);
    m_tableView->setWordWrap(false);
    m_tableView->setEditTriggers(QAbstractItemView::DoubleClicked |
                                 QAbstractItemView::EditKeyPressed |
                                 QAbstractItemView::SelectedClicked);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->verticalHeader()->setDefaultSectionSize(30);
    m_tableView->horizontalHeader()->setHighlightSections(false);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    // 自定义代理：负责性别、出生日期两列的编辑器
    PatientDelegate *delegate = new PatientDelegate(this);
    m_tableView->setItemDelegate(delegate);
    m_tableView->setToolTip("双击“性别”或“出生日期”单元格可直接编辑，其余字段请用“修改”按钮");

    layout->addWidget(m_tableView, 1);

    // ================= 底部状态行 =================
    m_lblStatus = new QLabel(this);
    m_lblStatus->setObjectName("HintLabel");
    layout->addWidget(m_lblStatus);

    // ================= 信号槽 =================
    connect(m_btnSearch, &QPushButton::clicked, this, &PatientManageWidget::onSearch);
    connect(m_editSearch, &QLineEdit::returnPressed, this, &PatientManageWidget::onSearch);
    connect(btnAdd, &QPushButton::clicked, this, &PatientManageWidget::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &PatientManageWidget::onEdit);
    connect(btnDelete, &QPushButton::clicked, this, &PatientManageWidget::onDelete);
    connect(btnRefresh, &QPushButton::clicked, this, &PatientManageWidget::onRefreshClicked);
    // 自定义代理编辑结束 → 提交数据
    connect(delegate, &QAbstractItemDelegate::closeEditor,
            this, &PatientManageWidget::onEditorClosed);
    reloadData();
}

// ---------------- 数据加载 ----------------

void PatientManageWidget::reloadData()
{
    m_currentKeyword.clear();
    m_editSearch->clear();
    if (m_model->filter().isEmpty())
        refreshTable();
    else
    {
        m_model->setFilter(QString());   // 清空过滤条件
        refreshTable();
    }
}

void PatientManageWidget::refreshTable()
{
    // setFilter 改变后必须 select() 重新查询，视图会自动更新
    const bool selectOk = m_model->select();
    if (!selectOk)
    {
        QMessageBox::warning(this, "读取失败",
                             "读取患者数据失败：" + m_model->lastError().text());
        return;
    }
    applyHeaderTitles();
    applyColumnWidths();
    updateStatus();
}

void PatientManageWidget::applyHeaderTitles()
{
    const QStringList titles = QStringList()
        << "患者编号" << "身份证号" << "姓名" << "性别" << "出生日期"
        << "身高(cm)" << "体重(kg)" << "手机号" << "年龄" << "建档时间";
    for (int i = 0; i < titles.size() && i < m_model->columnCount(); ++i)
        m_model->setHeaderData(i, Qt::Horizontal, titles.at(i));
}

void PatientManageWidget::applyColumnWidths()
{
    if (m_model->columnCount() == 0)
        return;
    m_tableView->setColumnWidth(PatientDelegate::ColId, 90);
    m_tableView->setColumnWidth(PatientDelegate::ColIdCard, 160);
    m_tableView->setColumnWidth(PatientDelegate::ColName, 80);
    m_tableView->setColumnWidth(PatientDelegate::ColSex, 60);
    m_tableView->setColumnWidth(PatientDelegate::ColDob, 100);
    m_tableView->setColumnWidth(PatientDelegate::ColHeight, 80);
    m_tableView->setColumnWidth(PatientDelegate::ColWeight, 80);
    m_tableView->setColumnWidth(PatientDelegate::ColMobile, 115);
    m_tableView->setColumnWidth(PatientDelegate::ColAge, 60);
    m_tableView->setColumnWidth(PatientDelegate::ColCreated, 145);
}

void PatientManageWidget::updateStatus()
{
    if (m_currentKeyword.isEmpty())
        m_lblStatus->setText(QString("共 %1 条患者记录").arg(m_model->rowCount()));
    else
        m_lblStatus->setText(QString("查询“%1”共 %2 条记录")
                             .arg(m_currentKeyword).arg(m_model->rowCount()));
}

// ---------------- 查询 ----------------

/*
 * 查找：姓名 / 身份证 / 手机号 模糊匹配
 * -------------------------------------------------------------
 * 1. 用 setFilter 而不是每次 setQuery，保持 View/Model 绑定关系；
 * 2. 关键字里的单引号要转义，避免 SQL 语法错误；
 * 3. 查不到时表格清空 + 状态提示 + 弹窗说明，界面状态合理。
 */
void PatientManageWidget::onSearch()
{
    const QString keyword = m_editSearch->text().trimmed();
    m_currentKeyword = keyword;

    if (keyword.isEmpty())
    {
        m_model->setFilter(QString());
        refreshTable();
        return;
    }

    QString escaped = keyword;
    escaped.replace("'", "''");   // 简单转义，避免引号截断 SQL

    m_model->setFilter(QString(
            "NAME LIKE '%%1%' OR ID_CARD LIKE '%%1%' OR MOBILEPHONE LIKE '%%1%'")
            .arg(escaped));

    if (!m_model->select())
    {
        QMessageBox::warning(this, "查询失败",
                             "查询语句执行失败：" + m_model->lastError().text());
        m_model->setFilter(QString());
        refreshTable();
        return;
    }

    applyHeaderTitles();
    applyColumnWidths();
    updateStatus();

    if (m_model->rowCount() == 0)
    {
        m_lblStatus->setText(QString("未找到与“%1”匹配的患者记录").arg(keyword));
        m_lblStatus->setStyleSheet("color:#DC2626;");
        QMessageBox::information(this, "查询结果",
                                 QString("未找到与“%1”匹配的患者记录。\n"
                                         "可以更换姓名、身份证或手机号再试。").arg(keyword));
    }
    else
    {
        m_lblStatus->setStyleSheet(QString());
    }
}

// ---------------- 新增 ----------------

void PatientManageWidget::onAdd()
{
    EditPatientDialog dlg(this);
    dlg.setMode(EditPatientDialog::AddMode);

    if (dlg.exec() != QDialog::Accepted)
        return;

    const PatientInfo info = dlg.patientInfo();
    QString message;
    if (!DBHelper::getInstance()->insertPatient(info, &message))
    {
        QMessageBox::warning(this, "新增失败", message);
        return;
    }

    DBHelper::getInstance()->logEvent(
                QString("新增患者 %1（%2）").arg(info.name, info.id));
    refreshTable();
    QMessageBox::information(this, "新增成功",
                             QString("患者 %1 已保存到数据库").arg(info.name));
}

// ---------------- 修改 ----------------

void PatientManageWidget::onEdit()
{
    const QString id = selectedPatientId();
    if (id.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先在表格中选中要修改的患者");
        return;
    }

    PatientInfo info;
    if (!DBHelper::getInstance()->loadPatient(id, &info))
    {
        QMessageBox::warning(this, "修改失败", "读取该患者信息失败，可能已被删除");
        refreshTable();
        return;
    }

    EditPatientDialog dlg(this);
    dlg.setMode(EditPatientDialog::EditMode);
    dlg.setPatientInfo(info);

    if (dlg.exec() != QDialog::Accepted)
        return;

    const PatientInfo newInfo = dlg.patientInfo();
    QString message;
    if (!DBHelper::getInstance()->updatePatient(newInfo, &message))
    {
        QMessageBox::warning(this, "修改失败", message);
        return;
    }

    DBHelper::getInstance()->logEvent(
                QString("修改患者 %1（%2）").arg(newInfo.name, newInfo.id));
    refreshTable();
    QMessageBox::information(this, "修改成功",
                             QString("患者 %1 的信息已更新").arg(newInfo.name));
}

// ---------------- 删除 ----------------

void PatientManageWidget::onDelete()
{
    const QString id = selectedPatientId();
    if (id.isEmpty())
    {
        QMessageBox::information(this, "提示", "请先在表格中选中要删除的患者");
        return;
    }

    const QString name = selectedPatientName();
    // 删除前必须二次确认
    if (QMessageBox::question(this, "删除确认",
                              QString("确定要删除患者【%1】（编号 %2）吗？\n"
                                      "删除后数据不可恢复。").arg(name, id),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

    QString message;
    if (!DBHelper::getInstance()->deletePatient(id, &message))
    {
        QMessageBox::warning(this, "删除失败", message);
        return;
    }

    DBHelper::getInstance()->logEvent(
                QString("删除患者 %1（%2）").arg(name, id));
    refreshTable();
    QMessageBox::information(this, "删除成功",
                             QString("患者 %1 已从数据库删除").arg(name));
}

void PatientManageWidget::onRefreshClicked()
{
    m_currentKeyword = m_editSearch->text().trimmed();
    m_lblStatus->setStyleSheet(QString());
    refreshTable();
}

/*
 * 代理编辑提交
 * -------------------------------------------------------------
 * QSqlTableModel 使用 OnManualSubmit 策略，单元格编辑后数据只在
 * Model 里标记为“脏数据”，必须 submitAll() 才真正写进数据库。
 * 提交失败时回滚，保证界面不会显示“假的成功”。
 */
void PatientManageWidget::onEditorClosed(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(editor)
    Q_UNUSED(hint)

    if (!m_model->isDirty())
        return;

    if (!m_model->submitAll())
    {
        QMessageBox::warning(this, "保存失败",
                             "修改未能写入数据库：" + m_model->lastError().text());
        m_model->revertAll();
        m_model->select();
        return;
    }

    DBHelper::getInstance()->logEvent("通过表格代理修改患者信息");
    m_lblStatus->setStyleSheet(QString());
    updateStatus();
}

// ---------------- 辅助 ----------------

QString PatientManageWidget::selectedPatientId() const
{
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid())
        return QString();
    return m_model->data(m_model->index(index.row(), PatientDelegate::ColId)).toString();
}

QString PatientManageWidget::selectedPatientName() const
{
    const QModelIndex index = m_tableView->currentIndex();
    if (!index.isValid())
        return QString();
    return m_model->data(m_model->index(index.row(), PatientDelegate::ColName)).toString();
}
