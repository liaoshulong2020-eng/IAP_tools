#include "siftdialog.h"

#include "mytablemodel.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

SiftDialog::SiftDialog(MyTableModel *model, int column, QWidget *parent)
    : QDialog(parent), model_(model), column_(column)
{
    setWindowTitle(tr("筛选"));

    values_ = model_->distinctValues(column_);
    const QSet<QString> current = model_->allowedValues(column_);

    auto *scroll = new QScrollArea(this);
    auto *container = new QWidget(scroll);
    auto *box = new QVBoxLayout(container);

    for (const QString &v : values_) {
        auto *cb = new QCheckBox(v, container);
        cb->setChecked(current.isEmpty() || current.contains(v));
        checks_.append(cb);
        box->addWidget(cb);
    }
    box->addStretch();
    scroll->setWidget(container);
    scroll->setWidgetResizable(true);

    auto *btnOk = new QPushButton(tr("确定"), this);
    btnOk->setObjectName(QStringLiteral("pushButton"));
    auto *btnAll = new QPushButton(tr("全选"), this);
    btnAll->setObjectName(QStringLiteral("pushButton_2"));
    auto *btnCancel = new QPushButton(tr("取消"), this);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(scroll);
    auto *buttons = new QHBoxLayout();
    buttons->addWidget(btnAll);
    buttons->addStretch();
    buttons->addWidget(btnOk);
    buttons->addWidget(btnCancel);
    QMetaObject::connectSlotsByName(this);
    layout->addLayout(buttons);
    resize(320, 400);
}

void SiftDialog::on_pushButton_clicked()
{
    QSet<QString> allowed;
    for (int i = 0; i < checks_.size(); ++i)
        if (checks_.at(i)->isChecked())
            allowed.insert(values_.at(i));
    model_->setColumnFilter(column_, allowed);
    accept();
}

void SiftDialog::on_pushButton_2_clicked()
{
    for (QCheckBox *cb : checks_)
        cb->setChecked(true);
}

// ---------------------------------------------------------------------------
// 静态便捷方法
// ---------------------------------------------------------------------------
int SiftDialog::chooseChannel(QWidget *parent)
{
    bool ok = false;
    const QString ch = QInputDialog::getItem(parent, QObject::tr("选择通道"),
                                             QObject::tr("通道："),
                                             { QObject::tr("通道1"), QObject::tr("通道2") },
                                             0, false, &ok);
    if (!ok)
        return -1;
    return ch == QObject::tr("通道2") ? 1 : 0;
}

void SiftDialog::filterColumn(QWidget *parent, MyTableModel *model, int column)
{
    if (column == MyTableModel::ColIndex)
        return;
    SiftDialog dlg(model, column, parent);
    dlg.exec();
}

void SiftDialog::showAllColumn(MyTableModel *model, int column)
{
    model->clearColumnFilter(column);
}

void SiftDialog::clearAllFilters(MyTableModel *model)
{
    model->clearFilters();
}
