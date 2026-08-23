#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>

class CanDevice;
class QLabel;

// 设备信息对话框
class INfDialog : public QDialog
{
    Q_OBJECT
public:
    explicit INfDialog(CanDevice *device, QWidget *parent = nullptr);

private slots:
    void on_pushButton_clicked();   // 刷新/关闭

private:
    void refresh();
    CanDevice *device_ = nullptr;
    QLabel *infoLabel_ = nullptr;
};

#endif // INFODIALOG_H
