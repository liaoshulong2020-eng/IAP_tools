#include "updatedlg.h"

#include "candevice.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

UpdateDlg::UpdateDlg(CanDevice *device, QWidget *parent)
    : QDialog(parent), device_(device)
{
    setWindowTitle(tr("固件升级"));

    pathEdit_ = new QLineEdit(this);
    // 默认固件路径
    pathEdit_->setText(QCoreApplication::applicationDirPath() + QStringLiteral("/firmware/firmware.bin"));

    auto *btnBrowse = new QPushButton(tr("浏览..."), this);
    btnBrowse->setObjectName(QStringLiteral("pushButton"));

    auto *row = new QHBoxLayout();
    row->addWidget(new QLabel(tr("固件文件："), this));
    row->addWidget(pathEdit_);
    row->addWidget(btnBrowse);

    auto *warning = new QLabel(tr("警告：升级过程中请勿关闭本界面或拔下设备。"), this);
    warning->setWordWrap(true);
    warning->setStyleSheet(QStringLiteral("color:#c00;"));

    auto *btnUpdate = new QPushButton(tr("升级"), this);
    btnUpdate->setObjectName(QStringLiteral("pushButton_2"));
    auto *btnClose = new QPushButton(tr("关闭"), this);
    btnClose->setObjectName(QStringLiteral("pushButton_3"));

    auto *buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(btnUpdate);
    buttons->addWidget(btnClose);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(row);
    layout->addWidget(warning);
    QMetaObject::connectSlotsByName(this);
    layout->addLayout(buttons);
    resize(520, 140);
}

void UpdateDlg::on_pushButton_clicked()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("选择固件"), QString(),
                                                      QStringLiteral("Firmware (*.bin *.hex)"));
    if (!path.isEmpty())
        pathEdit_->setText(path);
}

void UpdateDlg::on_pushButton_2_clicked()
{
    if (!device_->isOpen()) {
        QMessageBox::warning(this, tr("警告"), tr("设备未打开！"));
        return;
    }
    if (QMessageBox::question(this, tr("固件升级"),
                              tr("是否要升级固件？"))
        != QMessageBox::Yes)
        return;

    QString error;
    if (!device_->firmwareUpdate(pathEdit_->text(), &error)) {
        QMessageBox::warning(this, tr("警告"), error);
        return;
    }
    QMessageBox::information(this, tr("提示"),
                             tr("固件升级成功，请重新插拔设备并重启软件！"));
}

void UpdateDlg::on_pushButton_3_clicked()
{
    reject();
}
