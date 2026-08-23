#ifndef LISTSENDDLG_H
#define LISTSENDDLG_H

#include <QByteArray>
#include <QDialog>
#include <QList>

class CanDevice;
class QComboBox;
class QLineEdit;
class QTableWidget;
class QTimer;

// 列表发送对话框：多帧列表循环发送
class listSendDlg : public QDialog
{
    Q_OBJECT
public:
    explicit listSendDlg(CanDevice *device, QWidget *parent = nullptr);

    struct ListFrame {
        quint32 id = 0;
        bool extended = false;
        bool fd = false;
        QByteArray data;
    };

private slots:
    void on_pbfile_clicked();          // 从 CSV 加载
    void on_pbSend_clicked();          // 发送
    void on_pbStop_clicked();          // 停止
    void on_pbnClose_clicked();        // 关闭
    void on_comboBox_2_currentIndexChanged(int index);
    void onTimeout();

private:
    void addRow(const ListFrame &frame);
    QList<ListFrame> collectFrames() const;

    CanDevice *device_ = nullptr;
    int channel_ = 0;
    QComboBox *comboBox_ = nullptr;    // 通道
    QLineEdit *intervalEdit_ = nullptr;
    QTableWidget *table_ = nullptr;
    QTimer *timer_ = nullptr;
    int cursor_ = 0;
};

#endif // LISTSENDDLG_H
