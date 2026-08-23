#ifndef OPENDLG_H
#define OPENDLG_H

#include <QDialog>

class QComboBox;
class QSpinBox;

// 打开设备对话框：选择设备类型与索引号
class OpenDlg : public QDialog
{
    Q_OBJECT
public:
    explicit OpenDlg(QWidget *parent = nullptr);

    quint32 deviceType() const;
    quint32 deviceIndex() const;

    // 便捷静态方法：弹出并返回用户选择的设备类型/索引
    static bool getDevice(QWidget *parent, quint32 &type, quint32 &index);

private slots:
    void on_pushButton_clicked();   // OK
    void on_pushButton_2_clicked(); // Cancel

private:
    QComboBox *typeCombo_ = nullptr;
    QSpinBox *indexSpin_ = nullptr;
};

#endif // OPENDLG_H
