#include "widget.h"
#include "ui_widget.h"
#include <cmath>
#include <QKeyEvent>
#include <QPushButton>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setStyleSheet(R"(
    QWidget#Widget{
        background-color: #fafafa;
    }
    QLineEdit#display{
        background-color: #ffffff;
        border:1px solid #d8d9dc;
        border-radius:6px;
        font-size:36px;
        padding:14px;
        text-align:right;
    }
    QPushButton{
        font-size:20px;
        border-radius:4px;
        border:1px solid #dde0e4;
        min-height:48px;
    }
    QPushButton:hover{
        background-color:#d9dbe0;
    }
    /*数字与小数点按键：白色*/
    QPushButton#btnNum0,#btnNum1,#btnNum2,#btnNum3,#btnNum4,#btnNum5,#btnNum6,#btnNum7,#btnNum8,#btnNum9,#btnDot{
        background-color:#ffffff;
    }
    /*一元、二元运算符，CE C Del：浅灰色*/
    QPushButton#btnPercent,#btnReciprocal,#btnSquare,#btnRoot,#btnPlusMinus,#btnCE,#btnC,#btnDel,#btnAdd,#btnSub,#btnMul,#btnDiv{
        background-color:#e6e8eb;
    }
    /*等于按钮：加深，白色文字*/
    QPushButton#btnEqual{
        background-color:#546880;
        color:#ffffff;
        border:none;
    }
    )");




    connect(ui->btnNum0,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnNum1,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnNum2,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnNum3,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnNum4,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnNum5,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnNum6,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnNum7,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnNum8,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnNum9,SIGNAL(clicked()),this,SLOT(btnNumClick()));
    connect(ui->btnMul,SIGNAL(clicked()),this,SLOT(biOperaClicked()));
    connect(ui->btnDiv,SIGNAL(clicked()),this,SLOT(biOperaClicked()));
    connect(ui->btnAdd,SIGNAL(clicked()),this,SLOT(biOperaClicked()));
    connect(ui->btnSub,SIGNAL(clicked()),this,SLOT(biOperaClicked()));
    connect(ui->btnRoot,SIGNAL(clicked()),this,SLOT(siOperaClicked()));
    connect(ui->btnSquare,SIGNAL(clicked()),this,SLOT(siOperaClicked()));
    connect(ui->btnReciprocal,SIGNAL(clicked()),this,SLOT(siOperaClicked()));
    connect(ui->btnPercent,SIGNAL(clicked()),this,SLOT(siOperaClicked()));
    connect(ui->btnPlusMinus,SIGNAL(clicked()),this,SLOT(siOperaClicked()));

    // 键盘输入：由 Widget 统一接收按键，显示框和按钮都不抢占焦点
    ui->display->setFocusPolicy(Qt::NoFocus);
    for (QPushButton *button : findChildren<QPushButton *>())
        button->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();

    // Ctrl/Alt 组合键（如 Ctrl+C、Alt+F4）交还系统处理
    if (event->modifiers() & (Qt::ControlModifier | Qt::AltModifier)) {
        QWidget::keyPressEvent(event);
        return;
    }

    // 主键盘和小键盘的数字键使用同一套处理逻辑
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        inputDigit(QString::number(key - Qt::Key_0));
        return;
    }

    switch (key) {
    case Qt::Key_Period:
        inputDot();
        return;
    case Qt::Key_Plus:
        inputOperator("+");
        return;
    case Qt::Key_Minus:
        inputOperator("-");
        return;
    case Qt::Key_Asterisk:
        inputOperator("×");
        return;
    case Qt::Key_Slash:
        inputOperator("÷");
        return;
    case Qt::Key_Equal:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        equals();
        return;
    case Qt::Key_Backspace:
        backspace();
        return;
    case Qt::Key_Escape:
        clearAll();
        return;
    case Qt::Key_Delete:
        clearEntry();
        return;
    case Qt::Key_Percent:
        applyUnary("%");
        return;
    case Qt::Key_F9:
        applyUnary("±");
        return;
    default:
        break;
    }

    // 个别键盘布局下 % 通过字符形式传入
    if (event->text() == "%") {
        applyUnary("%");
        return;
    }

    QWidget::keyPressEvent(event);
}

bool Widget::calculation()
{
    if (pres.size() < 2 || opcodes.isEmpty())
        return false;

    const double num1 = pres[0].toDouble();
    const double num2 = pres[1].toDouble();
    const QString op = opcodes[0];
    double res = 0.0;

    if (op == "+")
        res = num1 + num2;
    else if (op == "-")
        res = num1 - num2;
    else if (op == "×")
        res = num1 * num2;
    else if (op == "÷") {
        if (num2 == 0.0) {
            showError();
            return false;
        }
        res = num1 / num2;
    } else {
        return false;
    }

    pres.clear();
    opcodes.clear();
    pres.push_back(QString::number(res));
    return true;
}

void Widget::inputDigit(const QString &digit)
{
    if (hasError)
        resetAll();

    // 结果或一元运算之后直接按数字：重新开始一个新数字，不再拼接结果
    if (startNewInput) {
        pre.clear();
        startNewInput = false;
    }

    // 避免 00、08 这类前导零，但保留 0. 和 -0
    if (pre == "0" || pre == "-0") {
        pre = (pre.startsWith('-') ? QString("-") : QString()) + digit;
    } else {
        pre += digit;
    }

    ui->display->setText(pre);
}

void Widget::inputOperator(const QString &op)
{
    if (hasError)
        resetAll();

    // 还没有输入操作数：连续按运算符时只替换最近一次，避免运算符堆积
    if (pre.isEmpty()) {
        if (!opcodes.isEmpty())
            opcodes.back() = op;
        return;
    }

    pres.push_back(pre);
    pre.clear();

    // 已凑齐两个操作数时先结算上一轮，把结果作为新的第一个操作数
    if (pres.size() >= 2) {
        if (!calculation())
            return;
        ui->display->setText(pres[0]);
    }

    opcodes.push_back(op);
    startNewInput = true;
}

void Widget::inputDot()
{
    if (hasError)
        resetAll();

    if (startNewInput) {
        pre.clear();
        startNewInput = false;
    }

    // 同一个操作数只允许出现一个小数点
    if (pre.contains('.'))
        return;

    if (pre.isEmpty())
        pre = "0";
    else if (pre == "-")
        pre = "-0";

    pre += ".";
    ui->display->setText(pre);
}

void Widget::backspace()
{
    if (hasError) {
        resetAll();
        return;
    }

    if (pre.isEmpty())
        return;

    // 直接修改成员 pre，保证内部状态与显示内容一致
    pre.chop(1);
    if (pre == "-")
        pre.clear();

    ui->display->setText(pre);
    startNewInput = false;
}

void Widget::equals()
{
    if (hasError)
        return;

    // 没有待完成的运算：保留当前值，下一次数字输入重新开始
    if (opcodes.isEmpty()) {
        if (!pre.isEmpty())
            startNewInput = true;
        return;
    }

    // 缺少第二个操作数时不计算
    if (pre.isEmpty())
        return;

    pres.push_back(pre);
    pre.clear();

    if (!calculation())
        return;

    pre = pres[0];
    ui->display->setText(pre);
    pres.clear();
    opcodes.clear();
    startNewInput = true;
}

void Widget::applyUnary(const QString &opText)
{
    if (hasError)
        resetAll();

    if (pre.isEmpty())
        return;

    if (opText == "±") {
        if (pre.startsWith('-'))
            pre.remove(0, 1);
        else
            pre.prepend('-');
        ui->display->setText(pre);
        startNewInput = true;
        return;
    }

    const double value = pre.toDouble();
    double res = 0.0;

    if (opText == "x²") {
        res = value * value;
    } else if (opText == "1/x") {
        if (value == 0.0) {
            showError();
            return;
        }
        res = 1.0 / value;
    } else if (opText == "√") {
        if (value < 0.0) {
            showError();
            return;
        }
        res = std::sqrt(value);
    } else if (opText == "%") {
        res = value / 100.0;
    } else {
        return;
    }

    pre = QString::number(res);
    ui->display->setText(pre);
    startNewInput = true;
}

void Widget::resetAll()
{
    pre.clear();
    pres.clear();
    opcodes.clear();
    startNewInput = false;
    hasError = false;
    ui->display->clear();
}

void Widget::showError()
{
    pre.clear();
    pres.clear();
    opcodes.clear();
    startNewInput = false;
    hasError = true;
    ui->display->setText("Error");
}

void Widget::btnNumClick()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button)
        inputDigit(button->text());
}

void Widget::biOperaClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button)
        inputOperator(button->text());
}

void Widget::siOperaClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button)
        applyUnary(button->text());
}

void Widget::on_btnDot_clicked()
{
    inputDot();
}

void Widget::on_btnDel_clicked()
{
    backspace();
}

void Widget::on_btnEqual_clicked()
{
    equals();
}

void Widget::on_btnC_clicked()
{
    clearAll();
}

void Widget::on_btnCE_clicked()
{
    clearEntry();
}

void Widget::clearAll()
{
    resetAll();
}

void Widget::clearEntry()
{
    if (hasError) {
        resetAll();
        return;
    }

    pre.clear();
    startNewInput = false;
    ui->display->clear();
}
