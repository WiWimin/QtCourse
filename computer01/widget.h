#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStack>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class QKeyEvent;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QString pre;                 // 当前输入的数字（或运算结果）
    QStack<QString> pres;        // 操作数栈
    QStack<QString> opcodes;     // 运算符栈
    bool startNewInput = false;  // 为 true 时，下一次数字输入重新开始
    bool hasError = false;       // 为 true 时，界面正在显示 Error

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void btnNumClick();
    void biOperaClicked();
    void siOperaClicked();
    void on_btnDot_clicked();

    void on_btnDel_clicked();

    void on_btnEqual_clicked();

    void on_btnC_clicked();

    void on_btnCE_clicked();

private:
    Ui::Widget *ui;
    QString lastExpression;  // 已完成的表达式，如 "12 + 3 ="

    // 鼠标按键与键盘事件共用的输入处理
    void inputDigit(const QString &digit);
    void inputOperator(const QString &op);
    void inputDot();
    void backspace();
    void clearAll();
    void clearEntry();
    void equals();
    void applyUnary(const QString &op);

    bool calculation();
    void resetAll();
    void showError();
    void updateExpression();
};
#endif // WIDGET_H
