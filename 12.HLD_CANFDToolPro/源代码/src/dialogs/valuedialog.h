#ifndef VALUEDIALOG_H
#define VALUEDIALOG_H

#include <QDialog>

class QSpinBox;

// 通用数值输入对话框
class valueDialog : public QDialog
{
    Q_OBJECT
public:
    valueDialog(const QString &title, const QString &label,
                int initial, int minimum, int maximum, QWidget *parent = nullptr);

    int value() const;
    int getValue() const { return value(); }

private slots:
    void on_btnValueOK_clicked();
    void on_btnValueCancel_clicked();

private:
    QSpinBox *spin_ = nullptr;
};

#endif // VALUEDIALOG_H
