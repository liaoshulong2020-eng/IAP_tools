#include "paradialog.h"

#include "baudrate.h"
#include "candevice.h"
#include "configmanager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ParaDialog::ParaDialog(CanDevice *device, int channel, QWidget *parent)
    : QDialog(parent), device_(device), channel_(channel)
{
    setWindowTitle(tr("参数设置"));

    auto *form = new QFormLayout();

    chCombo_ = new QComboBox(this);
    chCombo_->setObjectName(QStringLiteral("chCombo"));
    chCombo_->addItem(tr("通道1"), 0);
    chCombo_->addItem(tr("通道2"), 1);
    chCombo_->setCurrentIndex(channel);
    form->addRow(tr("通道："), chCombo_);

    abitCombo_ = new QComboBox(this);
    for (int i = 0; i < ABIT_BAUD_COUNT; ++i)
        abitCombo_->addItem(QString::fromLatin1(ABIT_BAUDS[i].label), ABIT_BAUDS[i].value);
    form->addRow(tr("仲裁域波特率："), abitCombo_);

    dbitCombo_ = new QComboBox(this);
    for (int i = 0; i < DBIT_BAUD_COUNT; ++i)
        dbitCombo_->addItem(QString::fromLatin1(DBIT_BAUDS[i].label), DBIT_BAUDS[i].value);
    form->addRow(tr("数据域波特率："), dbitCombo_);

    standardCombo_ = new QComboBox(this);
    standardCombo_->addItem(QStringLiteral("CANFD ISO"), 0);
    standardCombo_->addItem(QStringLiteral("CANFD BOSCH"), 1);
    form->addRow(tr("CAN-FD标准："), standardCombo_);

    modeCombo_ = new QComboBox(this);
    modeCombo_->addItem(tr("正常"), 0);
    modeCombo_->addItem(tr("只听"), 1);
    form->addRow(tr("工作模式："), modeCombo_);

    customCheck_ = new QCheckBox(tr("自定义波特率"), this);
    customCheck_->setObjectName(QStringLiteral("filterModeCheck"));
    customEdit_ = new QLineEdit(this);
    customEdit_->setPlaceholderText(tr("请使用波特率计算工具(baudcal)生成"));
    form->addRow(customCheck_, customEdit_);

    resistanceCheck_ = new QCheckBox(tr("使能终端电阻"), this);
    form->addRow(resistanceCheck_);

    loadConfig();

    auto *btnConfirm = new QPushButton(tr("确定"), this);
    btnConfirm->setObjectName(QStringLiteral("btnConfirm"));
    auto *btnSave = new QPushButton(tr("保存"), this);
    btnSave->setObjectName(QStringLiteral("btnSave"));
    auto *btnCancel = new QPushButton(tr("取消"), this);
    btnCancel->setObjectName(QStringLiteral("btnCancel"));

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    auto *buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(btnConfirm);
    buttons->addWidget(btnSave);
    QMetaObject::connectSlotsByName(this);
    buttons->addWidget(btnCancel);
    layout->addLayout(buttons);
}

void ParaDialog::apply()
{
    // 1. 保存到配置（始终生效）
    ConfigManager cfg;
    cfg.setValue(prefix() + QStringLiteral("paramABIT1"), QString::number(abitCombo_->currentIndex()));
    cfg.setValue(prefix() + QStringLiteral("paramABIT2"), QString::number(dbitCombo_->currentIndex()));
    cfg.setValue(prefix() + QStringLiteral("paramcanFDStandard"), QString::number(standardCombo_->currentIndex()));
    cfg.setValue(prefix() + QStringLiteral("paramworkStatus"), QString::number(modeCombo_->currentIndex()));
    cfg.setValue(prefix() + QStringLiteral("paramisCustomBaudrate"), customCheck_->isChecked() ? QStringLiteral("1") : QStringLiteral("0"));
    cfg.setValue(prefix() + QStringLiteral("paramCustomBaudrate"), customEdit_->text());
    cfg.save(ConfigManager::defaultFilePath());

    // 2. 应用到设备（仅当设备已打开）
    if (!device_->isOpen())
        return;

    QString error;
    if (customCheck_->isChecked()) {
        if (!device_->setCustomBaudrate(customEdit_->text(), &error)) {
            QMessageBox::warning(this, tr("警告"), error);
            return;
        }
    } else {
        const quint32 abit = static_cast<quint32>(abitCombo_->currentData().toUInt());
        const quint32 dbit = static_cast<quint32>(dbitCombo_->currentData().toUInt());
        if (!device_->setAbitBaud(abit, &error)) {
            QMessageBox::warning(this, tr("警告"), error);
            return;
        }
        if (!device_->setDbitBaud(dbit, &error)) {
            QMessageBox::warning(this, tr("警告"), error);
            return;
        }
    }
    if (!device_->setCanfdStandard(standardCombo_->currentIndex(), &error)) {
        QMessageBox::warning(this, tr("警告"), error);
        return;
    }
    if (!device_->setResistanceEnable(resistanceCheck_->isChecked(), &error)) {
        QMessageBox::warning(this, tr("警告"), error);
        return;
    }
}

QString ParaDialog::prefix() const
{
    return channel_ == 1 ? QStringLiteral("ch2") : QStringLiteral("ch1");
}

void ParaDialog::loadConfig()
{
    ConfigManager cfg;
    abitCombo_->setCurrentIndex(cfg.intValue(prefix() + QStringLiteral("paramABIT1"), 6));
    dbitCombo_->setCurrentIndex(cfg.intValue(prefix() + QStringLiteral("paramABIT2"), 3));
    standardCombo_->setCurrentIndex(cfg.intValue(prefix() + QStringLiteral("paramcanFDStandard"), 0));
    modeCombo_->setCurrentIndex(cfg.intValue(prefix() + QStringLiteral("paramworkStatus"), 0));
    customCheck_->setChecked(cfg.boolValue(prefix() + QStringLiteral("paramisCustomBaudrate"), false));
    customEdit_->setText(cfg.value(prefix() + QStringLiteral("paramCustomBaudrate")));
    customEdit_->setEnabled(customCheck_->isChecked());
}

void ParaDialog::on_btnConfirm_clicked()
{
    apply();
    accept();
}

void ParaDialog::on_btnSave_clicked()
{
    apply();
}

void ParaDialog::on_btnCancel_clicked()
{
    reject();
}

void ParaDialog::on_chCombo_currentIndexChanged(int index)
{
    channel_ = index;
    loadConfig();
}

void ParaDialog::on_filterModeCheck_clicked(bool checked)
{
    customEdit_->setEnabled(checked);
}
