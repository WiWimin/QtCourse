QT       += core gui widgets sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 本实验所有控件都用代码创建，不使用 .ui 文件
# （因此不再配置 FORMS，mainwindow.ui 保留但未参与编译）

SOURCES += \
    dbhelper.cpp \
    deptmanagewidget.cpp \
    doctordialog.cpp \
    doctormanagewidget.cpp \
    editpatientdialog.cpp \
    homewidget.cpp \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    patientdelegate.cpp \
    patientmanagewidget.cpp \
    registerdialog.cpp

HEADERS += \
    appstyle.h \
    dbhelper.h \
    deptmanagewidget.h \
    doctordialog.h \
    doctormanagewidget.h \
    editpatientdialog.h \
    homewidget.h \
    loginwidget.h \
    mainwindow.h \
    patientdelegate.h \
    patientmanagewidget.h \
    patienttablemodel.h \
    registerdialog.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
