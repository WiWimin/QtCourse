#include "appstyle.h"
#include "dbhelper.h"
#include "deptmanagewidget.h"
#include "doctormanagewidget.h"
#include "editpatientdialog.h"
#include "homewidget.h"
#include "loginwidget.h"
#include "mainwindow.h"
#include "patientmanagewidget.h"
#include "registerdialog.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QPixmap>
#include <QTextStream>

// 临时：把各页面渲染成 PNG，用于核对界面效果（HosipitalSystem.exe --shots）
static int runShots()
{
    if (!DBHelper::getInstance()->initDatabase())
        return 1;

    const QString dir = QApplication::applicationDirPath() + "/shots";
    QDir().mkpath(dir);

    auto shoot = [&dir](QWidget *w, const QString &name, bool isDialog = false) {
        if (isDialog)
        {
            // 对话框按布局推荐的宽高截取（不用 adjustSize，避免沿用默认 720 高）
            w->resize(w->sizeHint().width(), w->sizeHint().height());
        }
        else
            w->resize(1100, 720);     // 页面用统一窗口尺寸
        w->show();
        for (int i = 0; i < 6; ++i)
            QApplication::processEvents();
        w->grab().save(dir + "/" + name + ".png");
        w->hide();
    };

    LoginWidget login;
    shoot(&login, "01_登录页");

    HomeWidget home;
    shoot(&home, "02_欢迎页");

    PatientManageWidget patient;
    shoot(&patient, "03_患者管理页");

    DeptManageWidget dept;
    shoot(&dept, "04_科室管理页");

    DoctorManageWidget doctor;
    shoot(&doctor, "05_医生管理页");

    EditPatientDialog editDlg;
    shoot(&editDlg, "06_编辑患者对话框", true);

    RegisterDialog regDlg;
    shoot(&regDlg, "07_注册对话框", true);

    DBHelper::getInstance()->closeDatabase();
    return 0;
}

/*
 * 数据库业务自测模式（HosipitalSystem.exe --selftest）
 * -------------------------------------------------------------
 * 不弹界面，直接调用 DBHelper 的登录 / 新增 / 修改 / 删除接口，
 * 把结果写入 exe 同目录的 selftest_result.txt。
 * 用于开发阶段验证 SQL 与数据一致性，不影响正常启动。
 */
static int runSelfTest()
{
    QStringList output;
    bool allOk = true;

    // 记录一条检查结果
    auto check = [&output, &allOk](bool ok, const QString& name, const QString& detail = QString()) {
        output << QString("%1 %2%3")
                  .arg(ok ? "[通过]" : "[失败]")
                  .arg(name)
                  .arg(detail.isEmpty() ? QString() : QString("  -> ") + detail);
        if (!ok)
            allOk = false;
    };

    DBHelper *db = DBHelper::getInstance();
    check(db->initDatabase(), "数据库初始化（建表 + 默认数据）");

    // ---------- 1. 登录校验 ----------
    QString uid, uname, msg;
    check(db->checkLogin("admin", "123456", &uid, &uname, &msg),
          "正确用户名密码可以登录", uname);
    check(!db->checkLogin("admin", "badpwd", &uid, &uname, &msg),
          "错误密码被拒绝", msg);
    check(!db->checkLogin("no_such_user", "123456", &uid, &uname, &msg),
          "不存在的用户被拒绝", msg);

    // ---------- 2. 新增（含重复数据） ----------
    db->deletePatient("P900", &msg);   // 清理上次测试残留，不存在时返回 false 可忽略

    PatientInfo p;
    p.id = "P900";
    p.idCard = "441900200001019999";
    p.name = "自测患者";
    p.sex = 0;
    p.dob = "2000-01-01";
    p.height = 170.0;
    p.weight = 60.0;
    p.mobile = "13800138000";
    p.age = 26;
    p.createdTimestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    check(db->insertPatient(p, &msg), "新增患者 P900", msg);
    check(db->patientExists("P900"), "新增后数据库可以查到该记录");
    check(!db->insertPatient(p, &msg), "重复主键的新增被拒绝", msg);

    // 重复身份证（换主键）
    PatientInfo p2 = p;
    p2.id = "P901";
    check(!db->insertPatient(p2, &msg), "重复身份证号的新增被拒绝", msg);

    // ---------- 3. 修改 ----------
    PatientInfo edit = p;
    edit.name = "自测患者（已修改）";
    edit.mobile = "13900139000";
    edit.age = 27;
    check(db->updatePatient(edit, &msg), "修改患者信息", msg);

    PatientInfo loaded;
    const bool loadedOk = db->loadPatient("P900", &loaded);
    check(loadedOk && loaded.name == edit.name && loaded.mobile == edit.mobile,
          "修改后重新读取的数据一致",
          QString("%1 / %2").arg(loaded.name, loaded.mobile));

    // ---------- 4. 删除 ----------
    check(db->deletePatient("P900", &msg), "删除患者 P900", msg);
    check(!db->patientExists("P900"), "删除后数据库查不到该记录");
    check(!db->deletePatient("P900", &msg), "重复删除被拒绝", msg);

    // ---------- 5. 统计 ----------
    const int patientCount = db->queryInt("SELECT COUNT(*) FROM Patient");
    check(patientCount >= 3, "患者表数据统计正常",
          QString("当前 %1 条").arg(patientCount));

    // 写结果文件（程序是 Windows GUI 子系统，没有控制台，用文件输出）
    const QString path = QApplication::applicationDirPath() + "/selftest_result.txt";
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream ts(&file);
        ts.setCodec("UTF-8");
        ts.setGenerateByteOrderMark(true);
        ts << "医院诊疗系统 数据库自测\n";
        ts << "时间：" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
        ts << "====================================\n";
        foreach (const QString& line, output)
            ts << line << "\n";
        ts << "====================================\n";
        ts << (allOk ? "总体结果：全部通过\n" : "总体结果：存在失败项\n");
        file.close();
    }

    db->closeDatabase();
    return allOk ? 0 : 2;
}

/*
 * 程序入口
 * -------------------------------------------------------------
 * 1. 先初始化数据库（打开 + 建表 + 默认数据），失败就明确提示并退出；
 * 2. 再创建主窗口并进入事件循环；
 * 3. 事件循环结束后才关闭数据库。
 *
 * 注意：closeDatabase() 必须放在 a.exec() 之后。
 * 如果写在 exec() 之前，窗口一显示连接就被关闭，
 * 后续所有 QSqlQuery 都会报 “database not open”。
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("医院诊疗患者信息管理系统");
    a.setStyleSheet(appStyleSheet());

    // 临时截图模式
    if (a.arguments().contains("--shots"))
        return runShots();

    // 开发调试用的数据库自测模式：HosipitalSystem.exe --selftest
    if (a.arguments().contains("--selftest"))
        return runSelfTest();

    if (!DBHelper::getInstance()->initDatabase())
    {
        QMessageBox::critical(nullptr,
                              "启动失败",
                              "数据库初始化失败：无法创建或打开 hospital.db。\n"
                              "请确认程序所在目录有写入权限后重试。");
        return 1;
    }

    MainWindow w;
    w.show();

    const int ret = a.exec();

    DBHelper::getInstance()->closeDatabase();
    return ret;
}
