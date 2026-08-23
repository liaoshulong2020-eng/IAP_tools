#ifndef FILEDLG_H
#define FILEDLG_H

#include <QByteArray>
#include <QDialog>
#include <QList>

class CanDevice;
class QComboBox;
class QLineEdit;
class QTimer;

// 文件发送对话框：按 CSV 文件逐帧发送
class fileDlg : public QDialog
{
    Q_OBJECT
public:
    explicit fileDlg(CanDevice *device, QWidget *parent = nullptr);

    struct SendFrame {
        quint32 id = 0;
        bool extended = false;
        bool fd = false;
        QByteArray data;
    };

private slots:
    void on_pbfile_clicked();          // 选择文件
    void on_pbSend_clicked();
    void on_pbStop_clicked();
    void on_comboBox_2_currentIndexChanged(int index);
    void onTimeout();

private:
    CanDevice *device_ = nullptr;
    int channel_ = 0;
    QComboBox *comboBox_ = nullptr;
    QLineEdit *intervalEdit_ = nullptr;
    QLineEdit *fileEdit_ = nullptr;
    QTimer *timer_ = nullptr;
    QList<SendFrame> frames_;
    int cursor_ = 0;
};

#endif // FILEDLG_H
