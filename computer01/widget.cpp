#include "widget.h"
#include "ui_widget.h"
#include<cmath>
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
}

Widget::~Widget()
{
    delete ui;
}

void Widget::calculation()
{
    if(pres.size()<2||opcodes.isEmpty()){
        return;
    }
    double num1=pres[0].toDouble();
    double num2=pres[1].toDouble();
    double res=0;
    QString op=opcodes[0];
    if(op=="+")
        res=num1+num2;
    if(op=="-")
        res=num1-num2;
    if(op=="×")
        res=num1*num2;
    if(op=="÷"){
        if(num2==0){
            ui->display->setText("Error");
            pre="";
            pres.clear();
            opcodes.clear();
            return;
        }
        res=num1/num2;
    }
    pres.clear();
    opcodes.clear();
    pres.push_back(QString::number(res));
}

void Widget::btnNumClick()
{
QString digit=qobject_cast<QPushButton*>(sender())->text();
if(pre == "0")
    {
        pre = digit;
    }
    else
    {
        pre += digit;
    }
ui->display->setText(pre);
}

void Widget::biOperaClicked()
{
    QString opcode=qobject_cast<QPushButton*>(sender())->text();
    if(pre!=""){
        pres.push_back(pre);
        pre="";
    }
    if(pres.size()>=2){
        calculation();
        ui->display->setText(pres[0]);
    }
    opcodes.push_back(opcode);
}

void Widget::siOperaClicked()
{
    if(pre.isEmpty())
        return;
    QString opText=qobject_cast<QPushButton*>(sender())->text();
    double val=pre.toDouble();
    double res;
    if(opText=="x²"){
        res=val*val;
    }
    if(opText=="1/x"){
        if(val==0){
            ui->display->setText("Error");
            pre="";
            return;
        }
        res=1.0/val;
    }
    if(opText=="√"){
        if(val<0){
            ui->display->setText("Error");
            pre="";
            return;
        }
        res=sqrt(val);
    }
    if(opText=="±"){
        if(pre.startsWith("-"))
                    pre.remove(0,1);
                else
                    pre.prepend("-");
                ui->display->setText(pre);
                return;
    }
    if(opText=="%"){
        res=val/100.0;
    }
    pre=QString::number(res);
    ui->display->setText(pre);
}


void Widget::on_btnDot_clicked()
{
    if(!pre.contains(".")){
            if(pre.isEmpty())
                pre = "0";
            pre += ".";
            ui->display->setText(pre);
        }
}


void Widget::on_btnDel_clicked()
{
    QString pre=ui->display->text();
    pre=pre.left(pre.length()-1);
    ui->display->setText(pre);
}


void Widget::on_btnEqual_clicked()
{
    if(pre!=""){
        pres.push_back(pre);
        pre="";
    }
    if(pres.size()>=2){
        calculation();
        ui->display->setText(pres[0]);
        pre=pres[0];
        pres.clear();
        opcodes.clear();
    }
}


void Widget::on_btnC_clicked()
{
    pre = "";
        pres.clear();
        opcodes.clear();
        ui->display->setText("");
}


void Widget::on_btnCE_clicked()
{
    pre = "";
        ui->display->setText("");
}

