#include "mainwindow.h"

#include "dbhelper.h"
#include "doctormanagewidget.h"
#include "deptmanagewidget.h"
#include "homewidget.h"
#include "loginwidget.h"
#include "patientmanagewidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("医院诊疗患者信息管理系统");
    resize(920, 640);
    setMinimumSize(860, 560);

    buildToolBar();

    // 中央区域：页面栈，所有页面叠放在这里，只显示当前页
    m_stack = new QStackedWidget(this);
    m_stack->setObjectName("PageStack");
    setCentralWidget(m_stack);

    buildPages();

    // ---------- 状态栏：左侧学号姓名（实验要求），右侧当前登录用户 ----------
    QLabel *devInfo = new QLabel("学号：2024414290102　　姓名：陈福明", this);
    statusBar()->addWidget(devInfo);

    m_lblUser = new QLabel("当前用户：未登录", this);
    statusBar()->addPermanentWidget(m_lblUser);

    // 初始停留在登录页
    showPage(m_loginPage, "登录", false);
}

/*
 * 顶部工具栏：返回按钮 + 页面标题 + 退出登录
 * 全部用代码创建控件（实验要求“采用代码化方法添加控件”）
 */
void MainWindow::buildToolBar()
{
    m_toolBar = addToolBar("导航");
    m_toolBar->setObjectName("MainToolBar");
    m_toolBar->setMovable(false);
    m_toolBar->setFloatable(false);
    m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

    // 用一个水平布局容器让标题真正居中
    QWidget *holder = new QWidget(m_toolBar);
    holder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QHBoxLayout *h = new QHBoxLayout(holder);
    h->setContentsMargins(4, 0, 4, 0);
    h->setSpacing(8);

    m_btnBack = new QToolButton(holder);
    m_btnBack->setText("← 返回");
    m_btnBack->setFixedWidth(90);
    m_btnBack->setCursor(Qt::PointingHandCursor);

    m_lblTitle = new QLabel("登录", holder);
    m_lblTitle->setObjectName("PageTitle");
    m_lblTitle->setAlignment(Qt::AlignCenter);
    m_lblTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_btnLogout = new QToolButton(holder);
    m_btnLogout->setText("退出");
    m_btnLogout->setFixedWidth(90);
    m_btnLogout->setCursor(Qt::PointingHandCursor);

    h->addWidget(m_btnBack);
    h->addWidget(m_lblTitle, 1);
    h->addWidget(m_btnLogout);

    m_toolBar->addWidget(holder);

    connect(m_btnBack,   &QToolButton::clicked, this, &MainWindow::onBackClicked);
    connect(m_btnLogout, &QToolButton::clicked, this, &MainWindow::onLogoutClicked);
}

/*
 * 创建所有页面并接入信号
 */
void MainWindow::buildPages()
{
    m_loginPage  = new LoginWidget(this);
    m_homePage   = new HomeWidget(this);
    m_patientPage = new PatientManageWidget(this);
    m_deptPage    = new DeptManageWidget(this);
    m_doctorPage  = new DoctorManageWidget(this);

    m_stack->addWidget(m_loginPage);
    m_stack->addWidget(m_homePage);
    m_stack->addWidget(m_patientPage);
    m_stack->addWidget(m_deptPage);
    m_stack->addWidget(m_doctorPage);

    // 登录成功 → 进入主页，并记录当前用户
    connect(m_loginPage, &LoginWidget::loginSuccess, this, &MainWindow::onLoginSuccess);

    // 主页三个入口
    connect(m_homePage, &HomeWidget::openDepartmentManage, this, &MainWindow::showDepartmentPage);
    connect(m_homePage, &HomeWidget::openDoctorManage,     this, &MainWindow::showDoctorPage);
    connect(m_homePage, &HomeWidget::openPatientManage,    this, &MainWindow::showPatientPage);

    // 子页面里也可以放“返回”按钮，统一回主页
    connect(m_patientPage, &PatientManageWidget::goBackHome, this, &MainWindow::showHome);
    connect(m_deptPage,    &DeptManageWidget::goBackHome,    this, &MainWindow::showHome);
    connect(m_doctorPage,  &DoctorManageWidget::goBackHome,  this, &MainWindow::showHome);
}

void MainWindow::showPage(QWidget *page, const QString& title, bool canGoBack)
{
    m_stack->setCurrentWidget(page);
    m_lblTitle->setText(title);
    m_btnBack->setVisible(canGoBack);
}

void MainWindow::onLoginSuccess(const QString& userId, const QString& fullName)
{
    Q_UNUSED(userId)
    m_lblUser->setText(QString("当前用户：%1").arg(fullName));
    m_btnLogout->setText("退出登录");
    showPage(m_homePage, "欢迎", true);
}

void MainWindow::showHome()
{
    if (DBHelper::getInstance()->activeUserId().isEmpty())
    {
        // 未登录时不应该进入主页
        showPage(m_loginPage, "登录", false);
        return;
    }
    showPage(m_homePage, "欢迎", true);
}

void MainWindow::showDepartmentPage()
{
    m_deptPage->reload();
    showPage(m_deptPage, "科室管理", true);
}

void MainWindow::showDoctorPage()
{
    m_doctorPage->reload();
    showPage(m_doctorPage, "医生管理", true);
}

void MainWindow::showPatientPage()
{
    // 每次进入都重新读取数据库，保证界面与数据库一致
    m_patientPage->reloadData();
    showPage(m_patientPage, "患者管理", true);
}

/*
 * 返回：子页面回主页；主页返回等于退出登录；登录页不显示返回按钮
 */
void MainWindow::onBackClicked()
{
    QWidget *current = m_stack->currentWidget();
    if (current == m_loginPage)
        return;
    if (current == m_homePage)
    {
        onLogoutClicked();
        return;
    }
    showHome();
}

void MainWindow::onLogoutClicked()
{
    // 登录页上的“退出”= 关闭程序
    if (m_stack->currentWidget() == m_loginPage)
    {
        close();
        return;
    }

    if (QMessageBox::question(this, "退出登录",
                              "确定要退出当前账号吗？",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

    DBHelper::getInstance()->logEvent("退出登录");
    DBHelper::getInstance()->setActiveUser(QString(), QString());

    m_lblUser->setText("当前用户：未登录");
    m_btnLogout->setText("退出");
    m_loginPage->resetInput();
    showPage(m_loginPage, "登录", false);
}
