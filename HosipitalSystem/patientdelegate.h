#ifndef PATIENTDELEGATE_H
#define PATIENTDELEGATE_H

#include <QStyledItemDelegate>
#include <QDate>

/*
 * 患者表格的自定义代理（Delegate）
 * ------------------------------------------------------------------
 * Model/View 中，Delegate 负责“单元格怎么编辑、怎么显示”。
 * 本实验用它完成两件事：
 *   1. 性别列：编辑时弹出下拉框（男 / 女），而不是让用户手输 0/1；
 *   2. 出生日期列：编辑时弹出日期选择器，并同步重算年龄列。
 * 其余列不允许在表格内直接改，统一走“修改”对话框，保证校验完整。
 */
class PatientDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum Column
    {
        ColId = 0,          // 患者编号
        ColIdCard = 1,      // 身份证
        ColName = 2,        // 姓名
        ColSex = 3,         // 性别
        ColDob = 4,         // 出生日期
        ColHeight = 5,      // 身高
        ColWeight = 6,      // 体重
        ColMobile = 7,      // 手机号
        ColAge = 8,          // 年龄（由出生日期推算）
        ColCreated = 9      // 建档时间
    };

    explicit PatientDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
    // 由出生日期计算周岁
    static int ageFromDate(const QDate& birth);
};

#endif // PATIENTDELEGATE_H
