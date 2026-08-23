#include "filterdialog.h"

#include "candevice.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

FilterDialog::FilterDialog(CanDevice *device, int channel, QWidget *parent)
    : QDialog(parent), device_(device), channel_(channel)
{
    setWindowTitle(tr("通道%1 滤波设置").arg(channel + 1));

    auto *form = new QHBoxLayout();
    comboBox_ = new QComboBox(this);
    comboBox_->setObjectName(QStringLiteral("comboBox"));
    comboBox_->addItem(tr("标准帧"), 0);
    comboBox_->addItem(tr("扩展帧"), 1);
    form->addWidget(new QLabel(tr("帧类型："), this));
    form->addWidget(comboBox_);
    form->addWidget(new QLabel(tr("起始ID(0x)："), this));
    startEdit_ = new QLineEdit(this);
    startEdit_->setText(QStringLiteral("000"));
    form->addWidget(startEdit_);
    form->addWidget(new QLabel(tr("结束ID(0x)："), this));
    endEdit_ = new QLineEdit(this);
    endEdit_->setText(QStringLiteral("7FF"));
    form->addWidget(endEdit_);

    table_ = new QTableWidget(this);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({ tr("帧类型"), tr("Start ID"), tr("End ID") });
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);

    auto *btnAdd = new QPushButton(tr("添加"), this);
    btnAdd->setObjectName(QStringLiteral("pushButton"));
    auto *btnDel = new QPushButton(tr("删除"), this);
    btnDel->setObjectName(QStringLiteral("pushButton_2"));
    auto *btnApply = new QPushButton(tr("应用"), this);
    btnApply->setObjectName(QStringLiteral("pushButton_4"));

    auto *buttons = new QHBoxLayout();
    buttons->addWidget(btnAdd);
    buttons->addWidget(btnDel);
    buttons->addStretch();
    buttons->addWidget(btnApply);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(table_);
    QMetaObject::connectSlotsByName(this);
    layout->addLayout(buttons);
    resize(480, 360);
}

void FilterDialog::on_comboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
}

void FilterDialog::on_pushButton_clicked()
{
    bool okStart = false, okEnd = false;
    const quint32 start = startEdit_->text().toUInt(&okStart, 16);
    const quint32 end = endEdit_->text().toUInt(&okEnd, 16);
    if (!okStart || !okEnd) {
        QMessageBox::warning(this, tr("警告"), tr("ID格式错误。"));
        return;
    }
    if (end < start) {
        QMessageBox::warning(this, tr("警告"), tr("结束ID不能小于起始ID"));
        return;
    }
    FilterGroup g;
    g.mode = comboBox_->currentIndex();
    g.start = start;
    g.end = end;
    groups_.append(g);

    const int row = table_->rowCount();
    table_->insertRow(row);
    table_->setItem(row, 0, new QTableWidgetItem(comboBox_->currentText()));
    table_->setItem(row, 1, new QTableWidgetItem(QStringLiteral("0x%1").arg(start, 3, 16, QLatin1Char('0')).toUpper()));
    table_->setItem(row, 2, new QTableWidgetItem(QStringLiteral("0x%1").arg(end, 3, 16, QLatin1Char('0')).toUpper()));
}

void FilterDialog::on_pushButton_2_clicked()
{
    const int row = table_->currentRow();
    if (row < 0 || row >= groups_.size())
        return;
    table_->removeRow(row);
    groups_.removeAt(row);
}

void FilterDialog::on_pushButton_4_clicked()
{
    applyFilters();
}

void FilterDialog::applyFilters()
{
    QString error;
    if (!device_ || !device_->isChannelStarted(channel_)) {
        QMessageBox::warning(this, tr("警告"), tr("设备未打开！"));
        return;
    }

    if (groups_.isEmpty()) {
        // 清除滤波
        if (!device_->clearFilter(channel_, &error))
            QMessageBox::warning(this, tr("警告"), error);
        else
            QMessageBox::information(this, tr("提示"), tr("滤波已清除。"));
        return;
    }

    if (!device_->clearFilter(channel_, &error)) {
        QMessageBox::warning(this, tr("警告"), error);
        return;
    }
    for (const FilterGroup &g : groups_) {
        if (!device_->setFilterMode(channel_, g.mode, &error)
            || !device_->setFilterStartID(channel_, g.start, &error)
            || !device_->setFilterEndID(channel_, g.end, &error)) {
            QMessageBox::warning(this, tr("警告"), error);
            return;
        }
    }
    if (!device_->ackFilter(channel_, &error)) {
        QMessageBox::warning(this, tr("警告"), error);
        return;
    }
    QMessageBox::information(this, tr("提示"), tr("滤波已应用。"));
}
