#include "infodialog.h"

#include "candevice.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

static QString versionText(quint16 v)
{
    return QStringLiteral("V%1.%2").arg(v >> 8).arg(v & 0xFF);
}

INfDialog::INfDialog(CanDevice *device, QWidget *parent)
    : QDialog(parent), device_(device)
{
    setWindowTitle(tr("设备信息"));

    infoLabel_ = new QLabel(this);
    infoLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto *btnClose = new QPushButton(tr("关闭"), this);
    btnClose->setObjectName(QStringLiteral("pushButton"));

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(infoLabel_);
    auto *buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(btnClose);
    QMetaObject::connectSlotsByName(this);
    layout->addLayout(buttons);

    refresh();
}

void INfDialog::refresh()
{
    if (!device_ || !device_->isOpen()) {
        infoLabel_->setText(tr("设备未打开！"));
        return;
    }
    ZCAN_DEVICE_INFO info;
    QString error;
    if (!device_->getDeviceInfo(&info, &error)) {
        infoLabel_->setText(error);
        return;
    }

    QString html = QStringLiteral("<table>");
    auto row = [&](const QString &k, const QString &v) {
        html += QStringLiteral("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(k, v);
    };
    row(tr("硬件版本"), versionText(info.hw_Version));
    row(tr("固件版本"), versionText(info.fw_Version));
    row(tr("驱动版本"), versionText(info.dr_Version));
    row(tr("接口库版本"), versionText(info.in_Version));
    row(tr("通道数"), QString::number(info.can_Num));
    row(tr("序列号"), QString::fromLatin1(reinterpret_cast<const char *>(info.str_Serial_Num), 20).trimmed());
    row(tr("硬件类型"), QString::fromLatin1(reinterpret_cast<const char *>(info.str_hw_Type), 40).trimmed());
    html += QStringLiteral("</table>");
    infoLabel_->setText(html);
}

void INfDialog::on_pushButton_clicked()
{
    accept();
}
