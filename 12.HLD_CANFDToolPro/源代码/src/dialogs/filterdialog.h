#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QList>

class CanDevice;
class QComboBox;
class QLineEdit;
class QTableWidget;

// 滤波设置对话框：支持标准/扩展帧多组滤波
class FilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FilterDialog(CanDevice *device, int channel, QWidget *parent = nullptr);

    struct FilterGroup {
        int mode = 0;      // 0=标准帧 1=扩展帧
        quint32 start = 0;
        quint32 end = 0;
    };

private slots:
    void on_comboBox_currentIndexChanged(int index);
    void on_pushButton_clicked();    // 添加
    void on_pushButton_2_clicked();  // 删除
    void on_pushButton_4_clicked();  // 应用

private:
    void applyFilters();

    CanDevice *device_ = nullptr;
    int channel_ = 0;
    QComboBox *comboBox_ = nullptr;  // 帧类型（标准/扩展）
    QLineEdit *startEdit_ = nullptr;
    QLineEdit *endEdit_ = nullptr;
    QTableWidget *table_ = nullptr;
    QList<FilterGroup> groups_;
};

#endif // FILTERDIALOG_H
