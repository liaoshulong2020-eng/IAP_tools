#include "valuedialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

valueDialog::valueDialog(const QString &title, const QString &label,
                         int initial, int minimum, int maximum, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);

    auto *layout = new QVBoxLayout(this);
    auto *row = new QHBoxLayout();
    row->addWidget(new QLabel(label, this));
    spin_ = new QSpinBox(this);
    spin_->setRange(minimum, maximum);
    spin_->setValue(initial);
    row->addWidget(spin_);
    layout->addLayout(row);

    auto *btnOk = new QPushButton(tr("确定"), this);
    btnOk->setObjectName(QStringLiteral("btnValueOK"));
    auto *btnCancel = new QPushButton(tr("取消"), this);
    btnCancel->setObjectName(QStringLiteral("btnValueCancel"));
    auto *buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(btnOk);
    QMetaObject::connectSlotsByName(this);
    buttons->addWidget(btnCancel);
    layout->addLayout(buttons);
}

int valueDialog::value() const
{
    return spin_->value();
}

void valueDialog::on_btnValueOK_clicked()
{
    accept();
}

void valueDialog::on_btnValueCancel_clicked()
{
    reject();
}
