#ifndef SAVEFILEDLG_H
#define SAVEFILEDLG_H

#include <QDialog>

class QLineEdit;

// 实时保存对话框：选择 CSV 保存路径
class savefileDlg : public QDialog
{
    Q_OBJECT
public:
    explicit savefileDlg(QWidget *parent = nullptr);

    QString filePath() const;

private slots:
    void on_pushButton_3_clicked();   // 浏览

private:
    QLineEdit *pathEdit_ = nullptr;
};

#endif // SAVEFILEDLG_H
