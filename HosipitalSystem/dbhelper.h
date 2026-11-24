#ifndef DBHELPER_H
#define DBHELPER_H

#include <QSqlDatabase>
#include <QString>
#include <QStringList>
#include <QVariantList>

/*
 * 患者信息结构体
 * ------------------------------------------------------------------
 * MVC 中它属于 Model 层的数据载体：界面（View）只负责把控件内容
 * 装进这个结构体，再由 DBHelper 负责落库；界面不直接拼 SQL。
 */
struct PatientInfo
{
    QString id;                 // 患者编号（主键）
    QString idCard;             // 身份证号（唯一）
    QString name;               // 姓名
    int     sex;                // 性别：0=男，1=女
    QString dob;                // 出生日期，格式 yyyy-MM-dd
    double  height;             // 身高 cm
    double  weight;             // 体重 kg
    QString mobile;             // 手机号
    int     age;                // 年龄（由出生日期自动计算）
    QString createdTimestamp;   // 建档时间

    PatientInfo()
        : sex(0), height(0.0), weight(0.0), age(0)
    {
    }
};

/*
 * DBHelper —— 数据库单例工具类（MVC 的 Model 层）
 * ------------------------------------------------------------------
 * 1. 整个程序只保留一个数据库连接，连接名为 "hospital_conn"；
 * 2. 所有 SQL 都封装在本类中，界面层不出现 SQL 语句；
 * 3. 对外提供“业务方法”（登录校验、患者增删改查、日志），
 *    而不是把裸 SQL 暴露给界面。
 */
class DBHelper
{
public:
    // 获取全局唯一实例
    static DBHelper* getInstance();

    // 打开数据库 + 建表 + 写入初始数据；返回 false 表示失败
    bool initDatabase();
    // 关闭数据库连接
    void closeDatabase();
    // 供 QSqlTableModel 等模型使用的数据库连接对象
    QSqlDatabase& database();

    // ---------------- 通用 SQL 执行封装 ----------------
    // args 为空时直接执行；否则先 prepare，再按顺序绑定占位符 ?
    bool execSql(const QString& sql,
                 const QVariantList& args,
                 QString* message = nullptr);
    // 查询单个值（例如 SELECT COUNT(*) ...），失败返回 defaultValue
    int queryInt(const QString& sql,
                 const QVariantList& args = QVariantList(),
                 int defaultValue = 0);

    // ---------------- 用户（登录 / 注册） ----------------
    // 登录校验：区分“用户不存在”和“密码错误”，结果写入 message
    bool checkLogin(const QString& username,
                    const QString& password,
                    QString* userId,
                    QString* fullName,
                    QString* message);
    // 注册新用户
    bool registerUser(const QString& username,
                      const QString& password,
                      const QString& fullName,
                      QString* message);

    // ---------------- 患者（增删改查） ----------------
    bool patientExists(const QString& id);
    bool idCardExists(const QString& idCard, const QString& excludeId = QString());
    bool insertPatient(const PatientInfo& info, QString* message);
    bool updatePatient(const PatientInfo& info, QString* message);
    bool deletePatient(const QString& id, QString* message);
    bool loadPatient(const QString& id, PatientInfo* info);

    // ---------------- 科室（选做功能使用） ----------------
    QStringList departmentNames();
    QString departmentIdByName(const QString& name);
    QString nextDepartmentId();
    bool insertDepartment(const QString& id, const QString& name, QString* message);
    bool updateDepartment(const QString& id, const QString& name, QString* message);
    bool deleteDepartment(const QString& id, QString* message);
    int  doctorCountOfDepartment(const QString& departmentId);

    // ---------------- 医生（选做功能使用） ----------------
    QString nextDoctorId();
    bool doctorExists(const QString& id);
    bool insertDoctor(const QString& id,
                      const QString& employeeNo,
                      const QString& name,
                      const QString& departmentId,
                      QString* message);
    bool updateDoctor(const QString& id,
                      const QString& employeeNo,
                      const QString& name,
                      const QString& departmentId,
                      QString* message);
    bool deleteDoctor(const QString& id, QString* message);

    // ---------------- 当前登录用户 + 操作日志 ----------------
    void setActiveUser(const QString& id, const QString& name);
    QString activeUserId() const;
    QString activeUserName() const;
    // 记录一条操作日志到 History 表（选做功能）
    void logEvent(const QString& event);

private:
    DBHelper();
    ~DBHelper();

    bool createTables();
    bool initDefaultData();
    bool tableIsEmpty(const QString& tableName);

    static DBHelper* m_instance;   // 单例指针
    QSqlDatabase     m_db;         // 唯一数据库连接
    QString          m_activeUserId;
    QString          m_activeUserName;
};

#endif // DBHELPER_H
