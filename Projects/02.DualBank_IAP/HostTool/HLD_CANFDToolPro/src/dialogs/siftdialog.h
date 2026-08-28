#ifndef SIFTDIALOG_H
#define SIFTDIALOG_H

#include <QDialog>
#include <QSet>
#include <QStringList>

class MyTableModel;
class QCheckBox;

// 显示筛选对话框：按列值多选筛选（所有筛选条件为“与”关系）
class SiftDialog : public QDialog
{
    Q_OBJECT
public:
    SiftDialog(MyTableModel *model, int column, QWidget *parent = nullptr);

    // 便捷静态方法
    static int  chooseChannel(QWidget *parent);                 // 返回 0/1，取消返回 -1
    static void filterColumn(QWidget *parent, MyTableModel *model, int column);
    static void showAllColumn(MyTableModel *model, int column);
    static void clearAllFilters(MyTableModel *model);

private slots:
    void on_pushButton_clicked();    // OK
    void on_pushButton_2_clicked();  // 全选

private:
    MyTableModel *model_ = nullptr;
    int column_ = 0;
    QList<QCheckBox *> checks_;
    QStringList values_;
};

#endif // SIFTDIALOG_H
