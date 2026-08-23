#include "savefiledlg.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

savefileDlg::savefileDlg(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("实时保存"));

    pathEdit_ = new QLineEdit(this);
    auto *btnBrowse = new QPushButton(tr("浏览..."), this);
    btnBrowse->setObjectName(QStringLiteral("pushButton_3"));

    auto *row = new QHBoxLayout();
    row->addWidget(new QLabel(tr("保存路径："), this));
    row->addWidget(pathEdit_);
    row->addWidget(btnBrowse);

    auto *btnOk = new QPushButton(tr("开始"), this);
    auto *btnCancel = new QPushButton(tr("取消"), this);
    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    auto *buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(btnOk);
    buttons->addWidget(btnCancel);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(row);
    QMetaObject::connectSlotsByName(this);
    layout->addLayout(buttons);
    resize(420, 100);
}

QString savefileDlg::filePath() const
{
    return pathEdit_->text().trimmed();
}

void savefileDlg::on_pushButton_3_clicked()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("保存CSV"), QString(),
                                                      QStringLiteral("CSV Files (*.csv)"));
    if (!path.isEmpty())
        pathEdit_->setText(path);
}
