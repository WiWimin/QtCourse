#include "finddialog.h"
#include "ui_finddialog.h"

#include <QLineEdit>

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);

    // 在输入框中按回车，等价于点击“查找下一个”
    connect(ui->lineEditFind, &QLineEdit::returnPressed,
            this, &FindDialog::on_btnFindNext_clicked);
}

FindDialog::~FindDialog()
{
    delete ui;
}

QString FindDialog::findText() const
{
    return ui->lineEditFind->text();
}

bool FindDialog::isCaseSensitive() const
{
    return ui->checkCase->isChecked();
}

bool FindDialog::isBackward() const
{
    return ui->radioUp->isChecked();
}

void FindDialog::setFindText(const QString &text)
{
    ui->lineEditFind->setText(text);
    ui->lineEditFind->selectAll();
}

void FindDialog::on_btnFindNext_clicked()
{
    emit findNextRequested();
}
