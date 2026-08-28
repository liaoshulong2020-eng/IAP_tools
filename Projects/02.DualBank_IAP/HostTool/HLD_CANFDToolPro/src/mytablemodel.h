#ifndef MYTABLEMODEL_H
#define MYTABLEMODEL_H

#include "modelitem.h"

#include <QAbstractTableModel>
#include <QHash>
#include <QList>
#include <QSet>
#include <QVector>

// 收发表模型，11 列，底层数据为 QList<ModelItem>
class MyTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        ColIndex = 0,       // 序号
        ColSystemTime,      // 系统时间
        ColTimestamp,       // 时间标识
        ColChannel,         // 通道
        ColDirection,       // 收/发
        ColId,              // ID
        ColFrameType,       // 帧类型（标准/扩展/远程）
        ColDataFrame,       // 数据帧/远程帧
        ColDlc,             // 数据长度
        ColCanFd,           // CAN-FD
        ColData,            // 数据
        ColumnCount
    };

    explicit MyTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void append(const ModelItem &item);
    void appendBatch(const QList<ModelItem> &items);
    void clear();

    void setLimit(int limit);       // 界面最多显示帧数（listCount）
    void setMerge(bool merge);      // 合并列表数据

    int limit() const { return limit_; }
    bool merge() const { return merge_; }

    // 最近一帧的通道/ID，用于「检查接收ID」连续性校验
    quint32 lastId(int channel) const { return lastId_[channel]; }

    // 筛选：按列设置允许的显示值集合（空集合 = 清除该列筛选）
    void setColumnFilter(int column, const QSet<QString> &allowed);
    void clearColumnFilter(int column);
    void clearFilters();
    bool isColumnFiltered(int column) const { return filters_.contains(column); }

    // 某一列所有不同的显示值（供筛选对话框展示）
    QStringList distinctValues(int column) const;

    // 某列当前已筛选的值
    QSet<QString> allowedValues(int column) const { return filters_.value(column); }

private:
    void rebuildVisible();
    bool passesFilter(const ModelItem &item) const;
    QString displayValue(int column, const ModelItem &item) const;

    QList<ModelItem> items_;
    QVector<int> visible_;          // 可见行索引 -> items_ 下标
    QHash<int, QSet<QString>> filters_;  // 列 -> 允许值集合
    int limit_ = 1000;
    bool merge_ = false;
    quint64 sequenceBase_ = 0;      // 序号偏移（行被淘汰时累加）
    quint32 lastId_[2] = { 0xFFFFFFFF, 0xFFFFFFFF };
};

#endif // MYTABLEMODEL_H
