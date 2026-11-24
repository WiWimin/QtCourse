#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class QToolBar;
class QToolButton;
class QLabel;

class LoginWidget;
class HomeWidget;
class PatientManageWidget;
class DeptManageWidget;
class DoctorManageWidget;

/*
 * 主窗口 —— 控制器（Controller）
 * ------------------------------------------------------------------
 * 1. 用 QStackedWidget 管理所有页面，实现页面切换；
 * 2. 用 QToolBar 提供“返回 / 页面标题 / 退出登录”导航；
 * 3. 状态栏常驻显示开发者学号、姓名（实验硬性要求）；
 * 4. 连接各页面发出的信号，决定跳转到哪一页（界面与逻辑解耦）。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onLoginSuccess(const QString& userId, const QString& fullName);
    void onBackClicked();
    void onLogoutClicked();
    void showHome();
    void showDepartmentPage();
    void showDoctorPage();
    void showPatientPage();

private:
    void buildToolBar();
    void buildPages();
    // 切换页面并同步标题、返回按钮、退出按钮的状态
    void showPage(QWidget *page, const QString& title, bool canGoBack);

    QStackedWidget *m_stack;
    QToolBar      *m_toolBar;
    QToolButton   *m_btnBack;
    QToolButton   *m_btnLogout;
    QLabel        *m_lblTitle;
    QLabel        *m_lblUser;

    LoginWidget         *m_loginPage;
    HomeWidget          *m_homePage;
    PatientManageWidget *m_patientPage;
    DeptManageWidget    *m_deptPage;
    DoctorManageWidget  *m_doctorPage;
};

#endif // MAINWINDOW_H
