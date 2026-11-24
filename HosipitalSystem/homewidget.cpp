#include "homewidget.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

HomeWidget::HomeWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("Page");

    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(40, 30, 40, 40);
    outer->addStretch(1);

    // 顶部提示
    QLabel *hint = new QLabel("请选择要管理的模块", this);
    hint->setObjectName("HintLabel");
    hint->setAlignment(Qt::AlignCenter);
    QFont hintFont = hint->font();
    hintFont.setPointSize(11);
    hint->setFont(hintFont);

    // 三个入口按钮（代码化创建，不使用 .ui 文件）
    QPushButton *btnDept = new QPushButton("科室管理", this);
    QPushButton *btnDoctor = new QPushButton("医生管理", this);
    QPushButton *btnPatient = new QPushButton("患者管理", this);

    const QList<QPushButton*> buttons = QList<QPushButton*>()
                                        << btnDept << btnDoctor << btnPatient;
    foreach (QPushButton *btn, buttons)
    {
        btn->setObjectName("HomeButton");
        btn->setFixedSize(170, 140);
        btn->setCursor(Qt::PointingHandCursor);
    }

    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(28);
    btnRow->addStretch(1);
    btnRow->addWidget(btnDept);
    btnRow->addWidget(btnDoctor);
    btnRow->addWidget(btnPatient);
    btnRow->addStretch(1);

    outer->addWidget(hint);
    outer->addSpacing(36);
    outer->addLayout(btnRow);
    outer->addStretch(2);

    // 三个按钮分别发出不同信号，由 MainWindow 决定跳到哪一页
    connect(btnDept,    &QPushButton::clicked, this, &HomeWidget::openDepartmentManage);
    connect(btnDoctor,  &QPushButton::clicked, this, &HomeWidget::openDoctorManage);
    connect(btnPatient, &QPushButton::clicked, this, &HomeWidget::openPatientManage);
}
