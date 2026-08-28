#ifndef LISTSENDDLG_H
#define LISTSENDDLG_H

#include <QByteArray>
#include <QDialog>
#include <QList>
#include "modelitem.h"

class CanDevice;
class QComboBox;
class QLabel;
class QRadioButton;
class QTableWidget;
class QTimer;

// 序列发送对话框：可编辑多帧列表，单次或循环发送
class listSendDlg : public QDialog
{
    Q_OBJECT
public:
    explicit listSendDlg(CanDevice *device, int channel, bool fd, QWidget *parent = nullptr);
    void stopSending();

    struct ListFrame {
        quint32 id = 0;
        bool extended = false;
        bool fd = false;
        QByteArray data;
        int intervalMs = 10;
    };

signals:
    void frameSent(const ModelItem &item);
    void sequenceStarted(int channel, int frameCount, bool loop);
    void sequenceFinished(int channel);
    void sequenceStopped(int channel);

private slots:
    void loadCsv();
    void saveCsv();
    void addEmptyRow();
    void removeSelectedRows();
    void on_pbSend_clicked();          // 发送
    void on_pbStop_clicked();          // 停止
    void on_pbnClose_clicked();        // 关闭
    void on_comboBox_2_currentIndexChanged(int index);
    void onTimeout();

private:
    void addRow(const ListFrame &frame);
    QList<ListFrame> collectFrames() const;
    void renumberRows();
    void sendNext();
    bool parseRow(int row, ListFrame &frame, QString *error = nullptr) const;

    CanDevice *device_ = nullptr;
    int channel_ = 0;
    bool fd_ = false;
    QTableWidget *table_ = nullptr;
    QTimer *timer_ = nullptr;
    QRadioButton *onceRadio_ = nullptr;
    QRadioButton *loopRadio_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QList<ListFrame> sendingFrames_;
    int cursor_ = 0;
};

#endif // LISTSENDDLG_H
