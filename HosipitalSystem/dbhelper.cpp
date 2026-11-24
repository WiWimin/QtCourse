#include "dbhelper.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

// 数据库连接统一使用 Qt 默认连接，避免 QSqlQueryModel 找不到连接
DBHelper* DBHelper::m_instance = nullptr;

DBHelper* DBHelper::getInstance()
{
    // 第一次调用时创建，之后一直复用（懒汉式单例）
    if (m_instance == nullptr)
    {
        m_instance = new DBHelper();
    }
    return m_instance;
}

DBHelper::DBHelper()
{
    // 构造函数不打开数据库：打开动作放在 initDatabase() 里，
    // 这样“连接失败”可以返回给调用者并给出明确提示。
}

DBHelper::~DBHelper()
{
    closeDatabase();
}

QSqlDatabase& DBHelper::database()
{
    return m_db;
}

/*
 * 打开数据库 + 建表 + 初始化数据
 * -------------------------------------------------------------
 * 1. 数据库文件放在 exe 同目录，避免“从 Creator 运行”和
 *    “双击 exe 运行”时工作目录不同，导致打开到两个不同的库；
 * 2. 使用 Qt 的默认连接名（defaultConnection），后续所有 QSqlQuery 都绑定它；
 * 3. 第一次运行会自动建表，所以不需要手工建库。
 */
bool DBHelper::initDatabase()
{
    // 使用 Qt 的“默认连接”：这样 QSqlQueryModel 等不带连接参数的模型
    // 也能直接使用同一个数据库连接。
    // 本程序只在启动时调用一次 addDatabase，不会重复注册。
    const QString connName = QLatin1String(QSqlDatabase::defaultConnection);
    if (QSqlDatabase::contains(connName))
    {
        m_db = QSqlDatabase::database(connName);
    }
    else
    {
        m_db = QSqlDatabase::addDatabase("QSQLITE");
    }

    // 数据库文件：程序所在目录/hospital.db
    QString dbPath = QCoreApplication::applicationDirPath() + "/hospital.db";
    m_db.setDatabaseName(dbPath);

    if (!m_db.isOpen() && !m_db.open())
    {
        qCritical() << "数据库打开失败：" << m_db.lastError().text()
                    << " 路径：" << dbPath;
        return false;
    }

    // 打开外键约束（SQLite 默认关闭）
    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA foreign_keys = ON");

    if (!createTables())
        return false;

    // 首次运行写入默认账号和演示数据
    if (!initDefaultData())
        return false;

    qDebug() << "数据库初始化成功：" << dbPath;
    return true;
}

void DBHelper::closeDatabase()
{
    if (m_db.isOpen())
        m_db.close();
}

/*
 * 建表：字段严格按照实验报告给出的参考数据库设计
 * User(ID, FULLNAME, USERNAME, PASSWORD)
 * Patient(ID, ID_CARD, NAME, SEX, DOB, HEIGHT, WEIGHT, MOBILEPHONE, AGE, CREATEDTIMESTAMP)
 * Department(ID, NAME)
 * Doctor(ID, EMPLOYEEENO, NAME, DEPARTMENT_ID)
 * History(ID, USER_ID, EVENT, TIMESTAMP)
 */
bool DBHelper::createTables()
{
    QSqlQuery q(m_db);   // 所有 SQL 都显式绑定这个连接，避免落到别的连接上“表不存在”

    const QStringList createSqls = QStringList()
        // 1. 用户表
        << "CREATE TABLE IF NOT EXISTS User ("
           " ID TEXT PRIMARY KEY,"
           " FULLNAME TEXT,"
           " USERNAME TEXT UNIQUE NOT NULL,"
           " PASSWORD TEXT NOT NULL)"
        // 2. 患者表（本实验核心表）
        << "CREATE TABLE IF NOT EXISTS Patient ("
           " ID TEXT PRIMARY KEY,"
           " ID_CARD TEXT UNIQUE,"
           " NAME TEXT,"
           " SEX INTEGER,"
           " DOB TEXT,"
           " HEIGHT REAL,"
           " WEIGHT REAL,"
           " MOBILEPHONE TEXT,"
           " AGE INTEGER,"
           " CREATEDTIMESTAMP TEXT)"
        // 3. 科室表
        << "CREATE TABLE IF NOT EXISTS Department ("
           " ID TEXT PRIMARY KEY,"
           " NAME TEXT NOT NULL)"
        // 4. 医生表（DEPARTMENT_ID 外键指向科室）
        << "CREATE TABLE IF NOT EXISTS Doctor ("
           " ID TEXT PRIMARY KEY,"
           " EMPLOYEEENO TEXT,"
           " NAME TEXT,"
           " DEPARTMENT_ID TEXT,"
           " FOREIGN KEY(DEPARTMENT_ID) REFERENCES Department(ID))"
        // 5. 操作日志表（选做）
        << "CREATE TABLE IF NOT EXISTS History ("
           " ID TEXT PRIMARY KEY,"
           " USER_ID TEXT,"
           " EVENT TEXT,"
           " TIMESTAMP TEXT)";

    for (int i = 0; i < createSqls.size(); ++i)
    {
        if (!q.exec(createSqls.at(i)))
        {
            qCritical() << "建表失败（第" << i + 1 << "条）：" << q.lastError().text();
            return false;
        }
    }
    return true;
}

bool DBHelper::tableIsEmpty(const QString& tableName)
{
    return queryInt(QString("SELECT COUNT(*) FROM %1").arg(tableName), QVariantList(), -1) == 0;
}

/*
 * 初始化默认数据
 * -------------------------------------------------------------
 * admin / 123456 是登录测试账号；
 * 其余是演示用的科室、医生、患者数据，方便一运行就能看到表格内容。
 */
bool DBHelper::initDefaultData()
{
    // ---------- 默认登录账号 ----------
    if (queryInt("SELECT COUNT(*) FROM User WHERE USERNAME = ?",
                 QVariantList() << "admin") == 0)
    {
        QString msg;
        if (!execSql("INSERT INTO User(ID, FULLNAME, USERNAME, PASSWORD) VALUES(?,?,?,?)",
                     QVariantList() << "u001" << "系统管理员" << "admin" << "123456",
                     &msg))
        {
            qCritical() << "插入默认账号失败：" << msg;
            return false;
        }
    }

    // ---------- 科室 ----------
    if (tableIsEmpty("Department"))
    {
        QStringList deptArgs;
        const QStringList deptIds   = QStringList() << "D01" << "D02" << "D03" << "D04";
        const QStringList deptNames = QStringList() << "内科" << "外科" << "儿科" << "骨科";
        for (int i = 0; i < deptIds.size(); ++i)
        {
            QString msg;
            if (!execSql("INSERT INTO Department(ID, NAME) VALUES(?,?)",
                         QVariantList() << deptIds.at(i) << deptNames.at(i), &msg))
            {
                qCritical() << "插入科室失败：" << msg;
                return false;
            }
        }
    }

    // ---------- 医生 ----------
    if (tableIsEmpty("Doctor"))
    {
        QString msg;
        execSql("INSERT INTO Doctor(ID, EMPLOYEEENO, NAME, DEPARTMENT_ID) VALUES(?,?,?,?)",
                QVariantList() << "DR01" << "E1001" << "李明" << "D01", &msg);
        execSql("INSERT INTO Doctor(ID, EMPLOYEEENO, NAME, DEPARTMENT_ID) VALUES(?,?,?,?)",
                QVariantList() << "DR02" << "E1002" << "王芳" << "D03", &msg);
    }

    // ---------- 演示患者 ----------
    if (tableIsEmpty("Patient"))
    {
        const QString now = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        PatientInfo p;
        p.id = "P001"; p.idCard = "441900200001011234"; p.name = "张伟";
        p.sex = 0; p.dob = "2000-01-01"; p.height = 175.0; p.weight = 70.0;
        p.mobile = "13800138001"; p.age = 26; p.createdTimestamp = now;
        QString msg;
        if (!insertPatient(p, &msg))
            qWarning() << "插入演示患者失败：" << msg;

        p.id = "P002"; p.idCard = "441900199805204321"; p.name = "李静";
        p.sex = 1; p.dob = "1998-05-20"; p.height = 162.0; p.weight = 52.0;
        p.mobile = "13900139002"; p.age = 28; p.createdTimestamp = now;
        insertPatient(p, &msg);

        p.id = "P003"; p.idCard = "441900201003157777"; p.name = "王小虎";
        p.sex = 0; p.dob = "2010-03-15"; p.height = 150.0; p.weight = 42.0;
        p.mobile = "13700137003"; p.age = 16; p.createdTimestamp = now;
        insertPatient(p, &msg);
    }

    return true;
}

/*
 * 通用 SQL 执行
 * -------------------------------------------------------------
 * 统一做三件事：检查连接是否打开、prepare 预编译、exec 执行；
 * 参数按顺序绑定到 SQL 里的 ?，避免手工拼接字符串造成 SQL 注入。
 */
bool DBHelper::execSql(const QString& sql, const QVariantList& args, QString* message)
{
    if (message) message->clear();

    if (!m_db.isOpen())
    {
        if (message) *message = QStringLiteral("数据库连接未打开");
        return false;
    }

    QSqlQuery q(m_db);

    if (args.isEmpty())
    {
        // 没有参数时直接执行
        if (!q.exec(sql))
        {
            if (message) *message = q.lastError().text();
            qWarning() << "SQL 执行失败：" << sql << q.lastError().text();
            return false;
        }
        return true;
    }

    // 先预编译，再绑定参数
    if (!q.prepare(sql))
    {
        if (message) *message = q.lastError().text();
        return false;
    }
    for (int i = 0; i < args.size(); ++i)
        q.addBindValue(args.at(i));

    if (!q.exec())
    {
        if (message) *message = q.lastError().text();
        qWarning() << "SQL 执行失败：" << sql << q.lastError().text();
        return false;
    }
    return true;
}

int DBHelper::queryInt(const QString& sql, const QVariantList& args, int defaultValue)
{
    QSqlQuery q(m_db);
    if (!q.prepare(sql))
        return defaultValue;
    for (int i = 0; i < args.size(); ++i)
        q.addBindValue(args.at(i));
    if (!q.exec() || !q.next())
        return defaultValue;
    return q.value(0).toInt();
}

/*
 * 登录校验
 * -------------------------------------------------------------
 * 依次判断：SQL 是否执行成功 → 用户是否存在 → 密码是否匹配，
 * 因此界面能够区分“用户名不存在”和“密码错误”两种情况。
 */
bool DBHelper::checkLogin(const QString& username,
                          const QString& password,
                          QString* userId,
                          QString* fullName,
                          QString* message)
{
    if (message) message->clear();

    if (!m_db.isOpen())
    {
        if (message) *message = QStringLiteral("数据库连接未打开");
        return false;
    }

    QSqlQuery q(m_db);
    q.prepare("SELECT ID, FULLNAME, PASSWORD FROM User WHERE USERNAME = ?");
    q.addBindValue(username);

    if (!q.exec())
    {
        if (message) *message = QStringLiteral("查询异常：") + q.lastError().text();
        return false;
    }

    if (!q.next())
    {
        if (message) *message = QStringLiteral("用户名不存在，请检查输入");
        return false;
    }

    if (q.value(2).toString() != password)
    {
        if (message) *message = QStringLiteral("密码错误，请重新输入");
        return false;
    }

    if (userId)   *userId   = q.value(0).toString();
    if (fullName) *fullName = q.value(1).toString();
    return true;
}

bool DBHelper::registerUser(const QString& username,
                            const QString& password,
                            const QString& fullName,
                            QString* message)
{
    if (username.isEmpty() || password.isEmpty())
    {
        if (message) *message = QStringLiteral("用户名和密码不能为空");
        return false;
    }

    if (queryInt("SELECT COUNT(*) FROM User WHERE USERNAME = ?",
                 QVariantList() << username) > 0)
    {
        if (message) *message = QStringLiteral("用户名 %1 已存在").arg(username);
        return false;
    }

    QString id = QString("u%1").arg(QDateTime::currentMSecsSinceEpoch() % 1000000);
    return execSql("INSERT INTO User(ID, FULLNAME, USERNAME, PASSWORD) VALUES(?,?,?,?)",
                   QVariantList() << id << fullName << username << password,
                   message);
}

// ---------------- 患者相关 ----------------

bool DBHelper::patientExists(const QString& id)
{
    return queryInt("SELECT COUNT(*) FROM Patient WHERE ID = ?",
                    QVariantList() << id) > 0;
}

bool DBHelper::idCardExists(const QString& idCard, const QString& excludeId)
{
    QString sql = "SELECT COUNT(*) FROM Patient WHERE ID_CARD = ?";
    QVariantList args;
    args << idCard;
    // 修改场景需要排除自己这条记录
    if (!excludeId.isEmpty())
    {
        sql += " AND ID <> ?";
        args << excludeId;
    }
    return queryInt(sql, args) > 0;
}

bool DBHelper::insertPatient(const PatientInfo& info, QString* message)
{
    if (message) message->clear();

    if (patientExists(info.id))
    {
        if (message) *message = QStringLiteral("患者编号 %1 已存在，请更换").arg(info.id);
        return false;
    }
    if (idCardExists(info.idCard))
    {
        if (message) *message = QStringLiteral("身份证号 %1 已被其他患者使用").arg(info.idCard);
        return false;
    }

    return execSql("INSERT INTO Patient"
                   "(ID, ID_CARD, NAME, SEX, DOB, HEIGHT, WEIGHT, MOBILEPHONE, AGE, CREATEDTIMESTAMP)"
                   " VALUES(?,?,?,?,?,?,?,?,?,?)",
                   QVariantList() << info.id << info.idCard << info.name
                                  << info.sex << info.dob << info.height
                                  << info.weight << info.mobile << info.age
                                  << info.createdTimestamp,
                   message);
}

bool DBHelper::updatePatient(const PatientInfo& info, QString* message)
{
    if (message) message->clear();

    if (!patientExists(info.id))
    {
        if (message) *message = QStringLiteral("患者编号 %1 不存在，无法修改").arg(info.id);
        return false;
    }
    if (idCardExists(info.idCard, info.id))
    {
        if (message) *message = QStringLiteral("身份证号 %1 已被其他患者使用").arg(info.idCard);
        return false;
    }

    return execSql("UPDATE Patient SET"
                   " ID_CARD = ?, NAME = ?, SEX = ?, DOB = ?,"
                   " HEIGHT = ?, WEIGHT = ?, MOBILEPHONE = ?, AGE = ?, CREATEDTIMESTAMP = ?"
                   " WHERE ID = ?",
                   QVariantList() << info.idCard << info.name << info.sex << info.dob
                                  << info.height << info.weight << info.mobile
                                  << info.age << info.createdTimestamp << info.id,
                   message);
}

bool DBHelper::deletePatient(const QString& id, QString* message)
{
    if (message) message->clear();

    if (!patientExists(id))
    {
        if (message) *message = QStringLiteral("患者编号 %1 不存在，无法删除").arg(id);
        return false;
    }
    return execSql("DELETE FROM Patient WHERE ID = ?", QVariantList() << id, message);
}

bool DBHelper::loadPatient(const QString& id, PatientInfo* info)
{
    if (!info)
        return false;

    QSqlQuery q(m_db);
    q.prepare("SELECT ID, ID_CARD, NAME, SEX, DOB, HEIGHT, WEIGHT, MOBILEPHONE, AGE, CREATEDTIMESTAMP"
              " FROM Patient WHERE ID = ?");
    q.addBindValue(id);
    if (!q.exec() || !q.next())
        return false;

    info->id               = q.value(0).toString();
    info->idCard           = q.value(1).toString();
    info->name             = q.value(2).toString();
    info->sex              = q.value(3).toInt();
    info->dob              = q.value(4).toString();
    info->height           = q.value(5).toDouble();
    info->weight           = q.value(6).toDouble();
    info->mobile           = q.value(7).toString();
    info->age              = q.value(8).toInt();
    info->createdTimestamp = q.value(9).toString();
    return true;
}

// ---------------- 科室 ----------------

QStringList DBHelper::departmentNames()
{
    QStringList names;
    QSqlQuery q(m_db);
    if (q.exec("SELECT NAME FROM Department ORDER BY ID"))
    {
        while (q.next())
            names << q.value(0).toString();
    }
    return names;
}

QString DBHelper::departmentIdByName(const QString& name)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT ID FROM Department WHERE NAME = ?");
    q.addBindValue(name);
    if (q.exec() && q.next())
        return q.value(0).toString();
    return QString();
}

/*
 * 生成下一个科室编号：D01、D02 ……
 * 取现有编号里的最大数字 +1，保证不重复。
 */
QString DBHelper::nextDepartmentId()
{
    int maxNo = 0;
    QSqlQuery q(m_db);
    if (q.exec("SELECT ID FROM Department"))
    {
        while (q.next())
        {
            const QString id = q.value(0).toString();
            if (id.startsWith('D'))
            {
                bool ok = false;
                const int no = id.mid(1).toInt(&ok);
                if (ok && no > maxNo)
                    maxNo = no;
            }
        }
    }
    return QString("D%1").arg(maxNo + 1, 2, 10, QChar('0'));
}

bool DBHelper::insertDepartment(const QString& id, const QString& name, QString* message)
{
    if (name.trimmed().isEmpty())
    {
        if (message) *message = QStringLiteral("科室名称不能为空");
        return false;
    }
    if (queryInt("SELECT COUNT(*) FROM Department WHERE NAME = ?",
                 QVariantList() << name.trimmed()) > 0)
    {
        if (message) *message = QStringLiteral("科室“%1”已存在").arg(name.trimmed());
        return false;
    }
    return execSql("INSERT INTO Department(ID, NAME) VALUES(?,?)",
                   QVariantList() << id << name.trimmed(), message);
}

bool DBHelper::updateDepartment(const QString& id, const QString& name, QString* message)
{
    if (name.trimmed().isEmpty())
    {
        if (message) *message = QStringLiteral("科室名称不能为空");
        return false;
    }
    return execSql("UPDATE Department SET NAME = ? WHERE ID = ?",
                   QVariantList() << name.trimmed() << id, message);
}

bool DBHelper::deleteDepartment(const QString& id, QString* message)
{
    // 有医生挂在该科室时不允许删除，避免破坏外键关系
    if (doctorCountOfDepartment(id) > 0)
    {
        if (message)
            *message = QStringLiteral("该科室下还有医生，不能删除。请先调整医生所属科室。");
        return false;
    }
    return execSql("DELETE FROM Department WHERE ID = ?",
                   QVariantList() << id, message);
}

int DBHelper::doctorCountOfDepartment(const QString& departmentId)
{
    return queryInt("SELECT COUNT(*) FROM Doctor WHERE DEPARTMENT_ID = ?",
                    QVariantList() << departmentId);
}

bool DBHelper::doctorExists(const QString& id)
{
    return queryInt("SELECT COUNT(*) FROM Doctor WHERE ID = ?",
                    QVariantList() << id) > 0;
}

// 生成下一个医生编号：DR01、DR02 ……
QString DBHelper::nextDoctorId()
{
    int maxNo = 0;
    QSqlQuery q(m_db);
    if (q.exec("SELECT ID FROM Doctor"))
    {
        while (q.next())
        {
            const QString id = q.value(0).toString();
            if (id.startsWith("DR"))
            {
                bool ok = false;
                const int no = id.mid(2).toInt(&ok);
                if (ok && no > maxNo)
                    maxNo = no;
            }
        }
    }
    return QString("DR%1").arg(maxNo + 1, 2, 10, QChar('0'));
}

bool DBHelper::insertDoctor(const QString& id,
                            const QString& employeeNo,
                            const QString& name,
                            const QString& departmentId,
                            QString* message)
{
    if (id.trimmed().isEmpty() || name.trimmed().isEmpty())
    {
        if (message) *message = QStringLiteral("医生编号和姓名不能为空");
        return false;
    }
    if (doctorExists(id.trimmed()))
    {
        if (message) *message = QStringLiteral("医生编号 %1 已存在").arg(id.trimmed());
        return false;
    }
    return execSql("INSERT INTO Doctor(ID, EMPLOYEEENO, NAME, DEPARTMENT_ID) VALUES(?,?,?,?)",
                   QVariantList() << id.trimmed() << employeeNo.trimmed()
                                  << name.trimmed() << departmentId,
                   message);
}

bool DBHelper::updateDoctor(const QString& id,
                            const QString& employeeNo,
                            const QString& name,
                            const QString& departmentId,
                            QString* message)
{
    if (name.trimmed().isEmpty())
    {
        if (message) *message = QStringLiteral("医生姓名不能为空");
        return false;
    }
    if (!doctorExists(id))
    {
        if (message) *message = QStringLiteral("医生编号 %1 不存在").arg(id);
        return false;
    }
    return execSql("UPDATE Doctor SET EMPLOYEEENO = ?, NAME = ?, DEPARTMENT_ID = ? WHERE ID = ?",
                   QVariantList() << employeeNo.trimmed() << name.trimmed()
                                  << departmentId << id,
                   message);
}

bool DBHelper::deleteDoctor(const QString& id, QString* message)
{
    if (!doctorExists(id))
    {
        if (message) *message = QStringLiteral("医生编号 %1 不存在").arg(id);
        return false;
    }
    return execSql("DELETE FROM Doctor WHERE ID = ?", QVariantList() << id, message);
}

// ---------------- 当前用户与操作日志 ----------------

void DBHelper::setActiveUser(const QString& id, const QString& name)
{
    m_activeUserId   = id;
    m_activeUserName = name;
}

QString DBHelper::activeUserId() const
{
    return m_activeUserId;
}

QString DBHelper::activeUserName() const
{
    return m_activeUserName;
}

/*
 * 操作日志（选做功能）：每次增、删、改、登录都写一条记录，
 * 这样报告里可以展示 History 表的数据变化。
 */
void DBHelper::logEvent(const QString& event)
{
    // 用“时间戳 + 自增序号”保证主键唯一
    static int seq = 0;
    ++seq;
    QString id = QString("H%1%2")
                     .arg(QDateTime::currentMSecsSinceEpoch())
                     .arg(seq, 3, 10, QChar('0'));
    QString msg;
    execSql("INSERT INTO History(ID, USER_ID, EVENT, TIMESTAMP) VALUES(?,?,?,?)",
            QVariantList() << id
                           << (m_activeUserId.isEmpty() ? QString("system") : m_activeUserId)
                           << event
                           << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"),
            &msg);
}
