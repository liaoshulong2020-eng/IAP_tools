#ifndef DATADLG_H
#define DATADLG_H

#include <QDialog>

// 数据显示对话框（辅助信息展示）
class DataDlg : public QDialog
{
    Q_OBJECT
public:
    explicit DataDlg(const QString &text, QWidget *parent = nullptr);
};

#endif // DATADLG_H
