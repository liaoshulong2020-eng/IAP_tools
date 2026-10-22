#include "listsenddlg.h"

#include "candevice.h"

#include <QAbstractItemView>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <functional>

namespace {
enum Column { NumberColumn, IdColumn, DataColumn, IntervalColumn, ColumnCount };

QString cleanCsvCell(QByteArray value)
{
    QString text = QString::fromUtf8(value).trimmed();
    if (text.size() >= 2 && text.startsWith(QLatin1Char('"')) && text.endsWith(QLatin1Char('"')))
        text = text.mid(1, text.size() - 2);
    return text.trimmed();
}
}

listSendDlg::listSendDlg(CanDevice *device, int channel, bool fd, QWidget *parent)
    : QDialog(parent), device_(device), channel_(channel), fd_(fd)
{
    setWindowTitle(tr("通道%1 序列发送（%2）").arg(channel_ + 1).arg(fd_ ? QStringLiteral("CAN-FD") : QStringLiteral("CAN")));

    onceRadio_ = new QRadioButton(tr("单次"), this);
    loopRadio_ = new QRadioButton(tr("循环"), this);
    onceRadio_->setChecked(true);
    onceRadio_->setToolTip(tr("按顺序发送一轮后自动停止"));
    loopRadio_->setToolTip(tr("发送到末尾后从第一帧继续，直到手动停止"));

    auto *top = new QHBoxLayout();
    top->addWidget(new QLabel(tr("通道%1：%2").arg(channel_ + 1).arg(fd_ ? QStringLiteral("CAN-FD") : QStringLiteral("CAN")), this));
    top->addSpacing(18);
    top->addWidget(new QLabel(tr("发送模式："), this));
    top->addWidget(onceRadio_);
    top->addWidget(loopRadio_);
    top->addStretch();
    top->addWidget(new QLabel(tr("间隔 = 当前帧发出后，到下一帧的等待时间"), this));

    table_ = new QTableWidget(this);
    table_->setColumnCount(ColumnCount);
    table_->setHorizontalHeaderLabels({ tr("编号"), tr("ID(0x)"), tr("数据"), tr("间隔(ms)") });
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setSectionResizeMode(NumberColumn, QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(IdColumn, QHeaderView::Interactive);
    table_->setColumnWidth(IdColumn, 140);
    table_->horizontalHeader()->setSectionResizeMode(DataColumn, QHeaderView::Stretch);
    table_->horizontalHeader()->setSectionResizeMode(IntervalColumn, QHeaderView::ResizeToContents);

    auto *addButton = new QPushButton(tr("新增帧"), this);
    auto *removeButton = new QPushButton(tr("删除选中"), this);
    auto *loadButton = new QPushButton(tr("加载CSV"), this);
    auto *saveButton = new QPushButton(tr("保存CSV"), this);
    connect(addButton, &QPushButton::clicked, this, &listSendDlg::addEmptyRow);
    connect(removeButton, &QPushButton::clicked, this, &listSendDlg::removeSelectedRows);
    connect(loadButton, &QPushButton::clicked, this, &listSendDlg::loadCsv);
    connect(saveButton, &QPushButton::clicked, this, &listSendDlg::saveCsv);

    auto *editButtons = new QHBoxLayout();
    editButtons->addWidget(addButton);
    editButtons->addWidget(removeButton);
    editButtons->addWidget(loadButton);
    editButtons->addWidget(saveButton);
    editButtons->addStretch();

    auto *sendButton = new QPushButton(tr("发送"), this);
    auto *stopButton = new QPushButton(tr("停止"), this);
    auto *closeButton = new QPushButton(tr("关闭"), this);
    connect(sendButton, &QPushButton::clicked, this, &listSendDlg::on_pbSend_clicked);
    connect(stopButton, &QPushButton::clicked, this, &listSendDlg::on_pbStop_clicked);
    connect(closeButton, &QPushButton::clicked, this, &listSendDlg::on_pbnClose_clicked);
    statusLabel_ = new QLabel(tr("就绪"), this);
    auto *sendButtons = new QHBoxLayout();
    sendButtons->addWidget(statusLabel_);
    sendButtons->addStretch();
    sendButtons->addWidget(sendButton);
    sendButtons->addWidget(stopButton);
    sendButtons->addWidget(closeButton);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(top);
    layout->addLayout(editButtons);
    layout->addWidget(table_);
    layout->addLayout(sendButtons);
    resize(780, 500);

    timer_ = new QTimer(this);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &listSendDlg::sendNext);
    addEmptyRow();
}

void listSendDlg::on_comboBox_2_currentIndexChanged(int index)
{
    Q_UNUSED(index);
}

void listSendDlg::addRow(const ListFrame &frame)
{
    const int row = table_->rowCount();
    table_->insertRow(row);
    auto *number = new QTableWidgetItem(QString::number(row + 1));
    number->setFlags(number->flags() & ~Qt::ItemIsEditable);
    table_->setItem(row, NumberColumn, number);
    table_->setItem(row, IdColumn, new QTableWidgetItem(QStringLiteral("0x%1").arg(frame.id, 0, 16).toUpper()));
    table_->setItem(row, DataColumn, new QTableWidgetItem(QString::fromLatin1(frame.data.toHex(' ').toUpper())));
    table_->setItem(row, IntervalColumn, new QTableWidgetItem(QString::number(frame.intervalMs)));
}

void listSendDlg::addEmptyRow()
{
    ListFrame frame;
    frame.id = 0x100;
    frame.data = QByteArray(8, '\0');
    addRow(frame);
    table_->setCurrentCell(table_->rowCount() - 1, IdColumn);
}

void listSendDlg::removeSelectedRows()
{
    QSet<int> rows;
    for (const QModelIndex &index : table_->selectionModel()->selectedRows())
        rows.insert(index.row());
    QList<int> sorted = rows.values();
    std::sort(sorted.begin(), sorted.end(), std::greater<int>());
    for (int row : sorted) table_->removeRow(row);
    renumberRows();
}

void listSendDlg::renumberRows()
{
    for (int row = 0; row < table_->rowCount(); ++row)
        table_->item(row, NumberColumn)->setText(QString::number(row + 1));
}

bool listSendDlg::parseRow(int row, ListFrame &frame, QString *error) const
{
    auto text = [this, row](int column) {
        return table_->item(row, column) ? table_->item(row, column)->text().trimmed() : QString();
    };
    QString idText = text(IdColumn);
    if (idText.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) idText.remove(0, 2);
    bool ok = false;
    frame.id = idText.toUInt(&ok, 16);
    if (!ok || frame.id > 0x1FFFFFFFu) {
        if (error) *error = tr("第%1行 ID 无效").arg(row + 1);
        return false;
    }
    frame.extended = frame.id > 0x7FFu;
    frame.fd = fd_;

    QString dataText = text(DataColumn);
    dataText.remove(QLatin1Char(' '));
    dataText.remove(QLatin1Char('\t'));
    if (dataText.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) dataText.remove(0, 2);
    if (dataText.size() % 2 != 0) {
        if (error) *error = tr("第%1行数据必须是完整的十六进制字节").arg(row + 1);
        return false;
    }
    frame.data = QByteArray::fromHex(dataText.toLatin1());
    const int maximum = frame.fd ? 64 : 8;
    if (frame.data.size() > maximum) {
        if (error) *error = tr("第%1行数据超过%2字节").arg(row + 1).arg(maximum);
        return false;
    }
    frame.intervalMs = text(IntervalColumn).toInt(&ok);
    if (!ok || frame.intervalMs < 0) {
        if (error) *error = tr("第%1行间隔必须是大于等于0的整数").arg(row + 1);
        return false;
    }
    return true;
}

QList<listSendDlg::ListFrame> listSendDlg::collectFrames() const
{
    QList<ListFrame> frames;
    for (int row = 0; row < table_->rowCount(); ++row) {
        ListFrame frame;
        if (parseRow(row, frame)) frames.append(frame);
    }
    return frames;
}

void listSendDlg::loadCsv()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("加载序列"), QString(), tr("CSV 文件 (*.csv)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("加载失败"), file.errorString());
        return;
    }
    const QList<QByteArray> lines = file.readAll().split('\n');
    table_->setRowCount(0);
    for (const QByteArray &rawLine : lines) {
        const QByteArray line = rawLine.trimmed();
        if (line.isEmpty()) continue;
        const QList<QByteArray> columns = line.split(',');
        if (columns.size() < 2) continue;
        const bool oldSequenceFormat = columns.size() >= 5;
        const bool sequenceFormat = columns.size() >= 4;
        const int idColumn = sequenceFormat ? 1 : 0;
        const int dataColumn = oldSequenceFormat ? 3 : (sequenceFormat ? 2 : columns.size() - 1);
        const int intervalColumn = oldSequenceFormat ? 4 : (sequenceFormat ? 3 : -1);
        QString idText = cleanCsvCell(columns.at(idColumn));
        if (idText.contains(QStringLiteral("ID"), Qt::CaseInsensitive)) continue;

        ListFrame frame;
        if (idText.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) idText.remove(0, 2);
        bool ok = false;
        frame.id = idText.toUInt(&ok, 16);
        if (!ok) continue;
        frame.extended = frame.id > 0x7FFu;
        frame.fd = fd_;
        QString data = cleanCsvCell(columns.at(dataColumn));
        data.remove(QLatin1Char(' '));
        frame.data = QByteArray::fromHex(data.toLatin1());
        if (intervalColumn >= 0) {
            const int interval = cleanCsvCell(columns.at(intervalColumn)).toInt(&ok);
            if (ok && interval >= 0) frame.intervalMs = interval;
        }
        addRow(frame);
    }
    if (table_->rowCount() == 0) addEmptyRow();
}

void listSendDlg::saveCsv()
{
    const QString path = QFileDialog::getSaveFileName(this, tr("保存序列"), QStringLiteral("can_sequence.csv"), tr("CSV 文件 (*.csv)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("保存失败"), file.errorString());
        return;
    }
    file.write(QStringLiteral("编号,ID,数据,间隔(ms)\n").toUtf8());
    for (int row = 0; row < table_->rowCount(); ++row) {
        ListFrame frame;
        QString error;
        if (!parseRow(row, frame, &error)) {
            QMessageBox::warning(this, tr("无法保存"), error);
            return;
        }
        const QString line = QStringLiteral("%1,0x%2,\"%3\",%4\n")
            .arg(row + 1).arg(frame.id, 0, 16)
            .arg(QString::fromLatin1(frame.data.toHex(' ').toUpper())).arg(frame.intervalMs);
        file.write(line.toUtf8());
    }
}

void listSendDlg::on_pbSend_clicked()
{
    if (!device_->isChannelStarted(channel_)) {
        QMessageBox::warning(this, tr("无法发送"), tr("请先打开并启动通道%1。").arg(channel_ + 1));
        return;
    }
    sendingFrames_.clear();
    for (int row = 0; row < table_->rowCount(); ++row) {
        ListFrame frame;
        QString error;
        if (!parseRow(row, frame, &error)) {
            QMessageBox::warning(this, tr("序列有误"), error);
            return;
        }
        sendingFrames_.append(frame);
    }
    if (sendingFrames_.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("序列为空。"));
        return;
    }
    cursor_ = 0;
    timer_->stop();
    statusLabel_->setText(loopRadio_->isChecked() ? tr("循环发送中…") : tr("单次发送中…"));
    emit sequenceStarted(channel_, sendingFrames_.size(), loopRadio_->isChecked());
    hide();
    sendNext();
}

void listSendDlg::sendNext()
{
    if (sendingFrames_.isEmpty()) return;
    if (cursor_ >= sendingFrames_.size()) {
        if (!loopRadio_->isChecked()) {
            statusLabel_->setText(tr("单次发送已完成，共%1帧").arg(sendingFrames_.size()));
            emit sequenceFinished(channel_);
            return;
        }
        cursor_ = 0;
    }
    const ListFrame frame = sendingFrames_.at(cursor_++);
    QString error;
    const bool ok = frame.fd
        ? device_->sendCanFd(channel_, frame.id, frame.extended, false, false, frame.data, 0, &error)
        : device_->sendCan(channel_, frame.id, frame.extended, false, frame.data, 0, &error);
    if (!ok) {
        timer_->stop();
        statusLabel_->setText(tr("发送失败"));
        QMessageBox::warning(this, tr("发送失败"), error);
        return;
    }
    ModelItem item;
    item.systemTime = QDateTime::currentMSecsSinceEpoch();
    item.channel = channel_;
    item.transmit = true;
    item.fd = frame.fd;
    item.extended = frame.extended;
    item.id = frame.id;
    item.dlc = frame.data.size();
    item.data = frame.data;
    emit frameSent(item);

    if (cursor_ < sendingFrames_.size() || loopRadio_->isChecked()) {
        timer_->start(frame.intervalMs);
    } else {
        statusLabel_->setText(tr("单次发送已完成，共%1帧").arg(sendingFrames_.size()));
        emit sequenceFinished(channel_);
    }
}

void listSendDlg::on_pbStop_clicked()
{
    const bool wasRunning = timer_->isActive() || !sendingFrames_.isEmpty();
    timer_->stop();
    sendingFrames_.clear();
    statusLabel_->setText(tr("已停止"));
    if (wasRunning) emit sequenceStopped(channel_);
}

void listSendDlg::stopSending()
{
    on_pbStop_clicked();
}

void listSendDlg::on_pbnClose_clicked()
{
    on_pbStop_clicked();
    reject();
}

void listSendDlg::onTimeout()
{
    sendNext();
}
