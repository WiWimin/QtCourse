#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QStack>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    QString pre;
    QString opcode;
    QStack<QString> pres;
    QStack<QString>opcodes;

    void calculation();

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
};
#endif // WIDGET_H
