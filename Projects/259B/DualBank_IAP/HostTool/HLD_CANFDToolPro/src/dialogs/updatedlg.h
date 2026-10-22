#ifndef UPDATEDLG_H
#define UPDATEDLG_H

#include <QDialog>

class CanDevice;
class QLineEdit;

// 固件升级对话框
class UpdateDlg : public QDialog
{
    Q_OBJECT
public:
    explicit UpdateDlg(CanDevice *device, QWidget *parent = nullptr);

private slots:
    void on_pushButton_clicked();   // 浏览
    void on_pushButton_2_clicked(); // 升级
    void on_pushButton_3_clicked(); // 关闭

private:
    CanDevice *device_ = nullptr;
    QLineEdit *pathEdit_ = nullptr;
};

#endif // UPDATEDLG_H
