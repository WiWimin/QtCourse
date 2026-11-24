#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QWidget>

/*
 * 功能选择主页（View 层）
 * 三个入口：科室管理 / 医生管理 / 患者管理
 */
class HomeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HomeWidget(QWidget *parent = nullptr);

signals:
    void openDepartmentManage();
    void openDoctorManage();
    void openPatientManage();
};

#endif // HOMEWIDGET_H
