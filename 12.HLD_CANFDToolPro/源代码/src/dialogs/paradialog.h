#ifndef PARADIALOG_H
#define PARADIALOG_H

#include <QDialog>

class CanDevice;
class QComboBox;
class QCheckBox;
class QLineEdit;

// 参数设置对话框：仲裁/数据波特率、CANFD 标准、工作模式、自定义波特率、终端电阻
class ParaDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ParaDialog(CanDevice *device, int channel, QWidget *parent = nullptr);

private slots:
    void on_btnConfirm_clicked();
    void on_btnCancel_clicked();
    void on_chCombo_currentIndexChanged(int index);
    void on_btnSave_clicked();
    void on_filterModeCheck_clicked(bool checked);

private:
    void apply();
    void loadConfig();
    QString prefix() const;

    CanDevice *device_ = nullptr;
    int channel_ = 0;
    QComboBox *chCombo_ = nullptr;
    QComboBox *abitCombo_ = nullptr;
    QComboBox *dbitCombo_ = nullptr;
    QComboBox *standardCombo_ = nullptr;
    QComboBox *modeCombo_ = nullptr;
    QCheckBox *customCheck_ = nullptr;
    QLineEdit *customEdit_ = nullptr;
    QCheckBox *resistanceCheck_ = nullptr;
};

#endif // PARADIALOG_H
