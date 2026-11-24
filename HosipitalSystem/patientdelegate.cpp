#include "patientdelegate.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QModelIndex>

PatientDelegate::PatientDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

/*
 * 创建编辑器
 * -------------------------------------------------------------
 * 只有性别、出生日期两列返回编辑器，其它列返回 nullptr，
 * 这样表格里“双击乱改”被限制住，数据完整性由对话框保证。
 */
QWidget *PatientDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    Q_UNUSED(option)

    if (index.column() == ColSex)
    {
        QComboBox *box = new QComboBox(parent);
        box->addItem("男");
        box->addItem("女");
        // 选完立即提交，避免下拉框还开着但值没保存
        connect(box, QOverload<int>::of(&QComboBox::currentIndexChanged),
                const_cast<PatientDelegate*>(this), [this, box](int) {
            emit const_cast<PatientDelegate*>(this)->commitData(box);
        });
        return box;
    }

    if (index.column() == ColDob)
    {
        QDateEdit *edit = new QDateEdit(parent);
        edit->setCalendarPopup(true);
        edit->setDisplayFormat("yyyy-MM-dd");
        return edit;
    }

    return nullptr;
}

/*
 * 把 Model 里的数据放进编辑器
 */
void PatientDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == ColSex)
    {
        QComboBox *box = qobject_cast<QComboBox*>(editor);
        if (box)
        {
            const int sex = index.model()->data(index, Qt::EditRole).toInt();
            box->setCurrentIndex(sex == 0 ? 0 : 1);
        }
        return;
    }

    if (index.column() == ColDob)
    {
        QDateEdit *edit = qobject_cast<QDateEdit*>(editor);
        if (edit)
        {
            QDate date = QDate::fromString(
                        index.model()->data(index, Qt::EditRole).toString(),
                        "yyyy-MM-dd");
            if (!date.isValid())
                date = QDate(2000, 1, 1);
            edit->setDate(date);
        }
    }
}

/*
 * 把编辑器里的数据写回 Model
 * -------------------------------------------------------------
 * 出生日期改动后，同时把年龄列一起更新，
 * 避免出现“出生日期变了、年龄还是旧的”这种数据不一致。
 */
void PatientDelegate::setModelData(QWidget *editor,
                                   QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    if (index.column() == ColSex)
    {
        QComboBox *box = qobject_cast<QComboBox*>(editor);
        if (box)
            model->setData(index, box->currentIndex(), Qt::EditRole);
        return;
    }

    if (index.column() == ColDob)
    {
        QDateEdit *edit = qobject_cast<QDateEdit*>(editor);
        if (edit)
        {
            const QDate date = edit->date();
            model->setData(index, date.toString("yyyy-MM-dd"), Qt::EditRole);
            // 同步年龄列
            model->setData(model->index(index.row(), ColAge),
                           ageFromDate(date), Qt::EditRole);
        }
    }
}

void PatientDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

int PatientDelegate::ageFromDate(const QDate &birth)
{
    if (!birth.isValid())
        return 0;
    const QDate today = QDate::currentDate();
    int age = today.year() - birth.year();
    // 今年生日还没到，年龄减 1
    if (today.month() < birth.month() ||
        (today.month() == birth.month() && today.day() < birth.day()))
    {
        --age;
    }
    return age < 0 ? 0 : age;
}
