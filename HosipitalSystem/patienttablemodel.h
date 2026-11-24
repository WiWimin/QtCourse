#ifndef PATIENTTABLEMODEL_H
#define PATIENTTABLEMODEL_H

#include <QSqlTableModel>

/*
 * PatientTableModel —— 患者表的自定义 Model
 * -------------------------------------------------------------
 * 数据库里 SEX 字段存的是 0/1，直接显示在表格里可读性差。
 * 这里继承 QSqlTableModel，只重写 data()：把“性别”列（第 3 列，
 * 顺序为 编号/身份证/姓名/性别/出生日期/身高/体重/手机号/年龄/建档时间）
 * 的显示值换成“男/女”，其它列、其它角色一律交给父类处理，
 * 因此增删改查、setFilter、submitAll 等原有能力完全不受影响。
 */
class PatientTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    explicit PatientTableModel(QObject *parent, QSqlDatabase db)
        : QSqlTableModel(parent, db)
    {
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        // 只改“显示文本”，EditRole 仍是 0/1，代理里的下拉框可以直接用数字定位
        if (role == Qt::DisplayRole && index.column() == 3)
        {
            const int sex = QSqlTableModel::data(index, Qt::DisplayRole).toInt();
            return sex == 1 ? QStringLiteral("女") : QStringLiteral("男");
        }
        return QSqlTableModel::data(index, role);
    }
};

#endif // PATIENTTABLEMODEL_H
