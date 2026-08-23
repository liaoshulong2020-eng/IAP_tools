#include "initdlg.h"

#include "baudrate.h"
#include "candevice.h"
#include "configmanager.h"
#include "paradialog.h"
#include "filterdialog.h"
#include "opendlg.h"

#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

initDlg::initDlg(CanDevice *device, QWidget *parent)
    : QDialog(parent), device_(device)
{
    setWindowTitle(tr("设备操作"));

    statusLabel_ = new QLabel(tr("设备未打开。"), this);

    auto *btnOpen = new QPushButton(tr("打开设备"), this);
    btnOpen->setObjectName(QStringLiteral("pushButton_2"));
    auto *btnParam = new QPushButton(tr("参数设置"), this);
    btnParam->setObjectName(QStringLiteral("pushButton_3"));
    auto *btnFilter = new QPushButton(tr("滤波设置"), this);
    btnFilter->setObjectName(QStringLiteral("pushButton_4"));
    auto *btnStart1 = new QPushButton(tr("启动通道1"), this);
    btnStart1->setObjectName(QStringLiteral("pushButton_5"));
    auto *btnStop1 = new QPushButton(tr("停止通道1"), this);
    btnStop1->setObjectName(QStringLiteral("pushButton_6"));
    auto *btnStart2 = new QPushButton(tr("启动通道2"), this);
    btnStart2->setObjectName(QStringLiteral("pushButton_8"));
    auto *btnStop2 = new QPushButton(tr("停止通道2"), this);
    btnStop2->setObjectName(QStringLiteral("pushButton_9"));
    auto *btnClose = new QPushButton(tr("关闭"), this);
    btnClose->setObjectName(QStringLiteral("pushButton_7"));

    auto *grid = new QGridLayout(this);
    grid->addWidget(statusLabel_, 0, 0, 1, 4);
    grid->addWidget(btnOpen, 1, 0);
    grid->addWidget(btnParam, 1, 1);
    grid->addWidget(btnFilter, 1, 2);
    grid->addWidget(btnStart1, 2, 0, 1, 2);
    grid->addWidget(btnStop1, 2, 2, 1, 2);
    grid->addWidget(btnStart2, 3, 0, 1, 2);
    grid->addWidget(btnStop2, 3, 2, 1, 2);
    QMetaObject::connectSlotsByName(this);
    grid->addWidget(btnClose, 4, 0, 1, 4);

    refreshStatus();
}

void initDlg::refreshStatus()
{
    if (!device_->isOpen()) {
        statusLabel_->setText(tr("设备未打开。"));
        return;
    }
    const bool c1 = device_->isChannelStarted(0);
    const bool c2 = device_->isChannelStarted(1);
    statusLabel_->setText(tr("设备已打开。  通道1:%1  通道2:%2")
                          .arg(c1 ? tr("已启动") : tr("已停止"))
                          .arg(c2 ? tr("已启动") : tr("已停止")));
}

void initDlg::on_pushButton_2_clicked()
{
    quint32 type = ZCAN_USBCANFD_200U, index = 0;
    if (!OpenDlg::getDevice(this, type, index))
        return;
    QString error;
    if (!device_->open(type, index, &error)) {
        QMessageBox::warning(this, tr("警告"), error);
        return;
    }
    // 应用配置里保存的波特率 / CANFD 标准
    ConfigManager cfg;
    const int abitIdx = qBound(0, cfg.intValue(QStringLiteral("ch1paramABIT1"), 6), ABIT_BAUD_COUNT - 1);
    const int dbitIdx = qBound(0, cfg.intValue(QStringLiteral("ch1paramABIT2"), 3), DBIT_BAUD_COUNT - 1);
    const int standard = cfg.intValue(QStringLiteral("ch1paramcanFDStandard"), 0);
    device_->setAbitBaud(ABIT_BAUDS[abitIdx].value);
    device_->setDbitBaud(DBIT_BAUDS[dbitIdx].value);
    device_->setCanfdStandard(standard);
    refreshStatus();
}

void initDlg::on_pushButton_3_clicked()
{
    ParaDialog dlg(device_, 0, this);
    dlg.exec();
}

void initDlg::on_pushButton_4_clicked()
{
    FilterDialog dlg(device_, 0, this);
    dlg.exec();
}

void initDlg::startChannel(int channel)
{
    if (!device_->isOpen()) {
        QMessageBox::warning(this, tr("警告"), tr("设备未打开！"));
        return;
    }
    QString error;
    if (!device_->initChannel(channel, true, 0, &error)) {
        QMessageBox::warning(this, tr("警告"), error);
        return;
    }
    if (!device_->startChannel(channel, &error)) {
        QMessageBox::warning(this, tr("警告"), error);
        return;
    }
    refreshStatus();
}

void initDlg::stopChannel(int channel)
{
    QString error;
    device_->resetChannel(channel, &error);
    refreshStatus();
}

void initDlg::on_pushButton_5_clicked() { startChannel(0); }
void initDlg::on_pushButton_6_clicked() { stopChannel(0); }
void initDlg::on_pushButton_8_clicked() { startChannel(1); }
void initDlg::on_pushButton_9_clicked() { stopChannel(1); }
void initDlg::on_pushButton_7_clicked() { accept(); }
