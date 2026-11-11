#include "replacedialog.h"
#include "ui_replacedialog.h"

#include <QLineEdit>

ReplaceDialog::ReplaceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);

    // 回车默认执行“查找下一个”，避免误替换
    connect(ui->lineEditFind, &QLineEdit::returnPressed,
            this, &ReplaceDialog::on_btnFindNext_clicked);
    connect(ui->lineEditReplace, &QLineEdit::returnPressed,
            this, &ReplaceDialog::on_btnFindNext_clicked);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

QString ReplaceDialog::findText() const
{
    return ui->lineEditFind->text();
}

QString ReplaceDialog::replaceText() const
{
    return ui->lineEditReplace->text();
}

bool ReplaceDialog::isCaseSensitive() const
{
    return ui->checkCase->isChecked();
}

bool ReplaceDialog::isBackward() const
{
    return ui->radioUp->isChecked();
}

void ReplaceDialog::on_btnFindNext_clicked()
{
    emit findNextRequested();
}

void ReplaceDialog::on_btnReplace_clicked()
{
    emit replaceRequested();
}

void ReplaceDialog::on_btnReplaceAll_clicked()
{
    emit replaceAllRequested();
}
