#ifndef INITDLG_H
#define INITDLG_H

#include <QDialog>

class CanDevice;
class QLabel;
class QPushButton;

// 设备操作对话框：打开设备、启动/停止通道、参数与滤波设置入口
class initDlg : public QDialog
{
    Q_OBJECT
public:
    explicit initDlg(CanDevice *device, QWidget *parent = nullptr);

private slots:
    void on_pushButton_2_clicked();   // 打开设备
    void on_pushButton_3_clicked();   // 参数设置
    void on_pushButton_4_clicked();   // 滤波设置
    void on_pushButton_5_clicked();   // 启动通道1
    void on_pushButton_6_clicked();   // 停止通道1
    void on_pushButton_8_clicked();   // 启动通道2
    void on_pushButton_9_clicked();   // 停止通道2
    void on_pushButton_7_clicked();   // 关闭

private:
    void refreshStatus();
    void startChannel(int channel);
    void stopChannel(int channel);

    CanDevice *device_ = nullptr;
    QLabel *statusLabel_ = nullptr;
};

#endif // INITDLG_H
