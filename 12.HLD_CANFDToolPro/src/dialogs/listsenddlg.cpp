#include "listsenddlg.h"

#include "candevice.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

listSendDlg::listSendDlg(CanDevice *device, QWidget *parent)
    : QDialog(parent), device_(device)
{
    setWindowTitle(tr("列表发送"));

    comboBox_ = new QComboBox(this);
    comboBox_->setObjectName(QStringLiteral("comboBox_2"));
    comboBox_->addItem(tr("通道1"), 0);
    comboBox_->addItem(tr("通道2"), 1);
    intervalEdit_ = new QLineEdit(QStringLiteral("10"), this);
    intervalEdit_->setObjectName(QStringLiteral("interval"));

    auto *top = new QHBoxLayout();
    top->addWidget(new QLabel(tr("通道："), this));
    top->addWidget(comboBox_);
    top->addWidget(new QLabel(tr("间隔(ms)："), this));
    top->addWidget(intervalEdit_);
    top->addStretch();

    table_ = new QTableWidget(this);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({ tr("ID(0x)"), tr("类型"), tr("数据") });
    table_->horizontalHeader()->setStretchLastSection(true);

    auto *btnFile = new QPushButton(tr("加载CSV"), this);
    btnFile->setObjectName(QStringLiteral("pbfile"));
    auto *btnSend = new QPushButton(tr("发送"), this);
    btnSend->setObjectName(QStringLiteral("pbSend"));
    auto *btnStop = new QPushButton(tr("停止"), this);
    btnStop->setObjectName(QStringLiteral("pbStop"));
    auto *btnClose = new QPushButton(tr("关闭"), this);
    btnClose->setObjectName(QStringLiteral("pbnClose"));

    auto *buttons = new QHBoxLayout();
    buttons->addWidget(btnFile);
    buttons->addStretch();
    buttons->addWidget(btnSend);
    buttons->addWidget(btnStop);
    buttons->addWidget(btnClose);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(top);
    layout->addWidget(table_);
    layout->addLayout(buttons);
    resize(560, 420);

    timer_ = new QTimer(this);
    QMetaObject::connectSlotsByName(this);
    connect(timer_, &QTimer::timeout, this, &listSendDlg::onTimeout);
}

void listSendDlg::on_comboBox_2_currentIndexChanged(int index)
{
    channel_ = index;
}

void listSendDlg::addRow(const ListFrame &frame)
{
    const int row = table_->rowCount();
    table_->insertRow(row);
    table_->setItem(row, 0, new QTableWidgetItem(QStringLiteral("0x%1").arg(frame.id, 3, 16, QLatin1Char('0')).toUpper()));
    table_->setItem(row, 1, new QTableWidgetItem(frame.fd ? QStringLiteral("CAN-FD") : QStringLiteral("CAN")));
    table_->setItem(row, 2, new QTableWidgetItem(QString::fromLatin1(frame.data.toHex(' ').toUpper())));
}

QList<listSendDlg::ListFrame> listSendDlg::collectFrames() const
{
    QList<ListFrame> frames;
    for (int r = 0; r < table_->rowCount(); ++r) {
        const QString idText = table_->item(r, 0) ? table_->item(r, 0)->text() : QString();
        const QString type = table_->item(r, 1) ? table_->item(r, 1)->text() : QString();
        const QString dataText = table_->item(r, 2) ? table_->item(r, 2)->text() : QString();
        if (idText.isEmpty())
            continue;
        bool ok = false;
        ListFrame f;
        QString s = idText;
        s.remove(QStringLiteral("0x"));
        f.id = s.toUInt(&ok, 16);
        if (!ok)
            continue;
        f.extended = f.id > 0x7FF;
        f.fd = type.contains(QStringLiteral("FD"), Qt::CaseInsensitive);
        QString hex = dataText;
        hex.remove(QLatin1Char(' '));
        f.data = QByteArray::fromHex(hex.toLatin1());
        frames.append(f);
    }
    return frames;
}

void listSendDlg::on_pbfile_clicked()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("加载CSV"), QString(),
                                                      QStringLiteral("CSV Files (*.csv)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    // 简单解析：跳过表头，取 ID 与数据列
    const QList<QByteArray> lines = f.readAll().split('\n');
    for (int i = 1; i < lines.size(); ++i) {
        const QList<QByteArray> cols = lines.at(i).split(',');
        if (cols.size() < 2)
            continue;
        const QString idText = QString::fromLatin1(cols.at(0));
        bool ok = false;
        ListFrame frame;
        QString s = idText;
        s.remove(QStringLiteral("0x"));
        frame.id = s.toUInt(&ok, 16);
        if (!ok)
            continue;
        frame.extended = frame.id > 0x7FF;
        // 数据列在最后一列
        QString hex = QString::fromLatin1(cols.last());
        hex.remove(QLatin1Char('"'));
        hex.remove(QLatin1Char(' '));
        frame.data = QByteArray::fromHex(hex.toLatin1());
        addRow(frame);
    }
}

void listSendDlg::on_pbSend_clicked()
{
    const QList<ListFrame> frames = collectFrames();
    if (frames.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("列表为空。"));
        return;
    }
    cursor_ = 0;
    timer_->start(intervalEdit_->text().toInt());
}

void listSendDlg::on_pbStop_clicked()
{
    timer_->stop();
}

void listSendDlg::on_pbnClose_clicked()
{
    timer_->stop();
    reject();
}

void listSendDlg::onTimeout()
{
    const QList<ListFrame> frames = collectFrames();
    if (frames.isEmpty()) {
        timer_->stop();
        return;
    }
    const ListFrame &f = frames.at(cursor_ % frames.size());
    ++cursor_;
    QString err;
    if (f.fd)
        device_->sendCanFd(channel_, f.id, f.extended, false, false, f.data, 0, &err);
    else
        device_->sendCan(channel_, f.id, f.extended, false, f.data, 0, &err);
}
