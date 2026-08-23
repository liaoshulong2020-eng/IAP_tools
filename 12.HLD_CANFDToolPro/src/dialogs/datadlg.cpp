#include "datadlg.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

DataDlg::DataDlg(const QString &text, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("数据"));
    auto *label = new QLabel(text, this);
    label->setObjectName(QStringLiteral("TextLabel"));
    label->setWordWrap(true);
    auto *btnOk = new QPushButton(tr("确定"), this);
    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(label);
    auto *buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(btnOk);
    layout->addLayout(buttons);
}
