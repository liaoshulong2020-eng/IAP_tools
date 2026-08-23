#include "opendlg.h"
#include "inc/controlcanfd.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

OpenDlg::OpenDlg(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("打开设备"));

    auto *form = new QFormLayout();
    typeCombo_ = new QComboBox(this);
    typeCombo_->addItem(QStringLiteral("USBCANFD-200U"), int(ZCAN_USBCANFD_200U));
    typeCombo_->addItem(QStringLiteral("USBCANFD-100U"), int(ZCAN_USBCANFD_100U));
    typeCombo_->addItem(QStringLiteral("USBCANFD-MINI"), int(ZCAN_USBCANFD_MINI));
    form->addRow(tr("设备类型："), typeCombo_);

    indexSpin_ = new QSpinBox(this);
    indexSpin_->setRange(0, 31);
    form->addRow(tr("设备索引："), indexSpin_);

    auto *btnOk = new QPushButton(tr("确定"), this);
    btnOk->setObjectName(QStringLiteral("pushButton"));
    auto *btnCancel = new QPushButton(tr("取消"), this);
    btnCancel->setObjectName(QStringLiteral("pushButton_2"));

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    auto *buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(btnOk);
    QMetaObject::connectSlotsByName(this);
    buttons->addWidget(btnCancel);
    layout->addLayout(buttons);
}

quint32 OpenDlg::deviceType() const
{
    return static_cast<quint32>(typeCombo_->currentData().toInt());
}

quint32 OpenDlg::deviceIndex() const
{
    return static_cast<quint32>(indexSpin_->value());
}

bool OpenDlg::getDevice(QWidget *parent, quint32 &type, quint32 &index)
{
    OpenDlg dlg(parent);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    type = dlg.deviceType();
    index = dlg.deviceIndex();
    return true;
}

void OpenDlg::on_pushButton_clicked()
{
    accept();
}

void OpenDlg::on_pushButton_2_clicked()
{
    reject();
}
