#include "filedlg.h"

#include "candevice.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

fileDlg::fileDlg(CanDevice *device, QWidget *parent)
    : QDialog(parent), device_(device)
{
    setWindowTitle(tr("文件发送"));

    comboBox_ = new QComboBox(this);
    comboBox_->setObjectName(QStringLiteral("comboBox_2"));
    comboBox_->addItem(tr("通道1"), 0);
    comboBox_->addItem(tr("通道2"), 1);
    intervalEdit_ = new QLineEdit(QStringLiteral("10"), this);
    intervalEdit_->setObjectName(QStringLiteral("interval"));
    fileEdit_ = new QLineEdit(this);

    auto *btnFile = new QPushButton(tr("浏览..."), this);
    btnFile->setObjectName(QStringLiteral("pbfile"));

    auto *fileRow = new QHBoxLayout();
    fileRow->addWidget(new QLabel(tr("文件："), this));
    fileRow->addWidget(fileEdit_);
    fileRow->addWidget(btnFile);

    auto *top = new QHBoxLayout();
    top->addWidget(new QLabel(tr("通道："), this));
    top->addWidget(comboBox_);
    top->addWidget(new QLabel(tr("间隔(ms)："), this));
    top->addWidget(intervalEdit_);
    top->addStretch();

    auto *btnSend = new QPushButton(tr("发送"), this);
    btnSend->setObjectName(QStringLiteral("pbSend"));
    auto *btnStop = new QPushButton(tr("停止"), this);
    btnStop->setObjectName(QStringLiteral("pbStop"));

    auto *buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(btnSend);
    buttons->addWidget(btnStop);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(fileRow);
    layout->addLayout(top);
    layout->addLayout(buttons);
    resize(480, 160);

    timer_ = new QTimer(this);
    QMetaObject::connectSlotsByName(this);
    connect(timer_, &QTimer::timeout, this, &fileDlg::onTimeout);
}

void fileDlg::on_comboBox_2_currentIndexChanged(int index)
{
    channel_ = index;
}

void fileDlg::on_pbfile_clicked()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("选择CSV"), QString(),
                                                      QStringLiteral("CSV Files (*.csv)"));
    if (path.isEmpty())
        return;
    fileEdit_->setText(path);

    // 解析 CSV
    frames_.clear();
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    const QList<QByteArray> lines = f.readAll().split('\n');
    for (int i = 1; i < lines.size(); ++i) {
        const QList<QByteArray> cols = lines.at(i).split(',');
        if (cols.size() < 2)
            continue;
        bool ok = false;
        SendFrame frame;
        QString s = QString::fromLatin1(cols.at(0));
        s.remove(QStringLiteral("0x"));
        frame.id = s.toUInt(&ok, 16);
        if (!ok)
            continue;
        frame.extended = frame.id > 0x7FF;
        QString hex = QString::fromLatin1(cols.last());
        hex.remove(QLatin1Char('"'));
        hex.remove(QLatin1Char(' '));
        frame.data = QByteArray::fromHex(hex.toLatin1());
        frames_.append(frame);
    }
}

void fileDlg::on_pbSend_clicked()
{
    if (frames_.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请先加载CSV文件。"));
        return;
    }
    cursor_ = 0;
    timer_->start(intervalEdit_->text().toInt());
}

void fileDlg::on_pbStop_clicked()
{
    timer_->stop();
}

void fileDlg::onTimeout()
{
    if (frames_.isEmpty()) {
        timer_->stop();
        return;
    }
    const SendFrame &f = frames_.at(cursor_ % frames_.size());
    ++cursor_;
    QString err;
    if (f.fd)
        device_->sendCanFd(channel_, f.id, f.extended, false, false, f.data, 0, &err);
    else
        device_->sendCan(channel_, f.id, f.extended, false, f.data, 0, &err);
}
