#include "mytablemodel.h"

#include <QBrush>
#include <QColor>
#include <QDateTime>
#include <algorithm>

MyTableModel::MyTableModel(QObject *parent) : QAbstractTableModel(parent) {}

int MyTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : visible_.size();
}

int MyTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant MyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();
    static const QStringList headers = {
        tr("序号"), tr("系统时间"), tr("时间标识"), tr("通道"), tr("方向"),
        tr("ID"), tr("帧类型"), tr("数据帧"), tr("长度"), tr("CAN-FD"), tr("数据"),
    };
    if (section < 0 || section >= ColumnCount)
        return QVariant();
    QString text = headers.at(section);
    if (isColumnFiltered(section))
        text += QLatin1Char('*');
    return text;
}

QString MyTableModel::displayValue(int column, const ModelItem &it) const
{
    switch (column) {
    case ColIndex:
        return QString();  // 序号列不参与筛选/取值
    case ColSystemTime:
        return QDateTime::fromMSecsSinceEpoch(it.systemTime).toString(QStringLiteral("hh:mm:ss.zzz"));
    case ColTimestamp:
        return QString::number(it.timestamp);
    case ColChannel:
        return QString::number(it.channel + 1);
    case ColDirection:
        return it.transmit ? QStringLiteral("TX") : QStringLiteral("RX");
    case ColId:
        return QStringLiteral("0x%1").arg(it.id, 0, 16).toUpper();
    case ColFrameType:
        return it.remote ? tr("远程帧") : (it.extended ? tr("扩展帧") : tr("标准帧"));
    case ColDataFrame:
        return it.remote ? tr("远程帧") : tr("数据帧");
    case ColDlc:
        return QString::number(it.dlc);
    case ColCanFd:
        if (!it.fd)
            return QStringLiteral("CAN");
        return it.brs ? QStringLiteral("CANFD-加速") : QStringLiteral("CANFD");
    case ColData:
        return QString::fromLatin1(it.data.toHex(' ').toUpper());
    default:
        return QString();
    }
}

QVariant MyTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= visible_.size())
        return QVariant();

    const ModelItem &it = items_.at(visible_.at(index.row()));

    if (role == Qt::BackgroundRole) {
        // 与 v1.21.0 完全一致的 RX 行底色。
        if (!it.transmit)
            return QBrush(QColor(QStringLiteral("#ECEFF3")));
        return QBrush(Qt::white);
    }

    if (role == Qt::TextAlignmentRole)
        return QVariant(int(Qt::AlignCenter));

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == ColIndex)
        return QString::number(sequenceBase_ + quint64(index.row()) + 1);
    return displayValue(index.column(), it);
}

void MyTableModel::append(const ModelItem &item)
{
    appendBatch(QList<ModelItem>() << item);
}

void MyTableModel::appendBatch(const QList<ModelItem> &items)
{
    if (items.isEmpty())
        return;

    QList<ModelItem> incoming = items;

    if (merge_ && filters_.isEmpty()) {
        // v1.21：哈希定位后只更新变化行，禁止每个刷新周期重置整张表。
        auto key = [](const ModelItem &item) {
            return (quint64(item.channel & 1) << 63)
                | (quint64(item.transmit) << 62)
                | (quint64(item.fd) << 61)
                | (quint64(item.extended) << 60)
                | quint64(item.id);
        };
        QHash<quint64, int> positions;
        positions.reserve(items_.size() + incoming.size());
        for (int i = 0; i < items_.size(); ++i) positions.insert(key(items_[i]), i);
        QList<ModelItem> additions;
        QVector<int> changed;
        for (ModelItem &item : incoming) {
            const quint64 frameKey = key(item);
            const auto found = positions.constFind(frameKey);
            if (found != positions.cend()) {
                const int row = found.value();
                if (row < items_.size()) {
                    items_[row] = std::move(item);
                    changed.push_back(row);
                } else {
                    // 同一个 UI 刷新批次内可能收到许多相同 ID 的 RX，只保留最新一帧。
                    additions[row - items_.size()] = std::move(item);
                }
            } else {
                positions.insert(frameKey, items_.size() + additions.size());
                additions.push_back(std::move(item));
            }
        }
        if (items_.size() + additions.size() > limit_) {
            beginResetModel();
            items_ += additions;
            const int remove = items_.size() - limit_;
            items_.erase(items_.begin(), items_.begin() + remove);
            sequenceBase_ += quint64(remove);
            rebuildVisible();
            endResetModel();
        } else {
            if (!additions.isEmpty()) {
                const int first = items_.size();
                beginInsertRows(QModelIndex(), first, first + additions.size() - 1);
                items_ += additions;
                // rowCount() 依赖 visible_，必须在 endInsertRows() 前同步更新，
                // 否则视图仍认为行数未增加，RX 虽已接收却不会显示。
                visible_.resize(items_.size());
                for (int i = 0; i < visible_.size(); ++i) visible_[i] = i;
                endInsertRows();
            }
            std::sort(changed.begin(), changed.end());
            for (int row : changed)
                emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
        }
    } else if (merge_) {
        // 有筛选时保持筛选语义，数据量受显示上限约束。
        for (ModelItem &item : incoming) {
            int found = -1;
            for (int i = 0; i < items_.size(); ++i)
                if (items_.at(i).sameFrame(item)) { found = i; break; }
            if (found >= 0) items_[found] = std::move(item); else items_.append(std::move(item));
        }
        if (items_.size() > limit_) {
            const int remove = items_.size() - limit_;
            items_.erase(items_.begin(), items_.begin() + remove);
            sequenceBase_ += quint64(remove);
        }
        beginResetModel(); rebuildVisible(); endResetModel();
    } else if (filters_.isEmpty()) {
        // 普通追加（无筛选）：增量 insertRows，避免整表重绘
        if (incoming.size() > limit_) {
            const int drop = incoming.size() - limit_;
            incoming = incoming.mid(drop);
            sequenceBase_ += quint64(drop);
        }
        const int remove = int(qMax<qsizetype>(0, items_.size() + incoming.size() - limit_));
        if (remove > 0) {
            beginRemoveRows(QModelIndex(), 0, remove - 1);
            items_.erase(items_.begin(), items_.begin() + remove);
            visible_.remove(0, remove);
            for (int i = 0; i < visible_.size(); ++i) visible_[i] = i;
            sequenceBase_ += quint64(remove);
            endRemoveRows();
        }
        const int first = items_.size();
        if (!incoming.isEmpty()) {
            beginInsertRows(QModelIndex(), first, first + incoming.size() - 1);
            items_ += incoming;
            visible_.resize(items_.size());
            for (int i = 0; i < visible_.size(); ++i) visible_[i] = i;
            endInsertRows();
        }
    } else {
        // 普通追加（有筛选）：整表重绘 + 重建可见行
        if (incoming.size() > limit_) {
            const int drop = incoming.size() - limit_;
            incoming = incoming.mid(drop);
            sequenceBase_ += quint64(drop);
        }
        const int remove = int(qMax<qsizetype>(0, items_.size() + incoming.size() - limit_));
        if (remove > 0) {
            items_.erase(items_.begin(), items_.begin() + remove);
            sequenceBase_ += quint64(remove);
        }
        items_ += incoming;
        beginResetModel();
        rebuildVisible();
        endResetModel();
    }

    // 更新「检查接收ID」使用的最近 ID
    if (!items_.isEmpty()) {
        const ModelItem &last = items_.last();
        if (!last.transmit)
            lastId_[last.channel] = last.id;
    }
}

void MyTableModel::clear()
{
    if (items_.isEmpty())
        return;
    beginResetModel();
    sequenceBase_ += quint64(items_.size());
    items_.clear();
    visible_.clear();
    lastId_[0] = lastId_[1] = 0xFFFFFFFF;
    endResetModel();
}

void MyTableModel::setLimit(int limit)
{
    limit_ = qMax(1, limit);
    const int remove = int(qMax<qsizetype>(0, items_.size() - limit_));
    if (remove > 0) {
        beginResetModel();
        items_.erase(items_.begin(), items_.begin() + remove);
        sequenceBase_ += quint64(remove);
        rebuildVisible();
        endResetModel();
    }
}

void MyTableModel::setMerge(bool merge)
{
    merge_ = merge;
}

// ---------------------------------------------------------------------------
// 筛选
// ---------------------------------------------------------------------------
void MyTableModel::setColumnFilter(int column, const QSet<QString> &allowed)
{
    if (column < 0 || column >= ColumnCount)
        return;
    if (allowed.isEmpty())
        filters_.remove(column);
    else
        filters_[column] = allowed;
    beginResetModel();
    rebuildVisible();
    endResetModel();
}

void MyTableModel::clearColumnFilter(int column)
{
    if (filters_.remove(column) == 0)
        return;
    beginResetModel();
    rebuildVisible();
    endResetModel();
}

void MyTableModel::clearFilters()
{
    if (filters_.isEmpty())
        return;
    filters_.clear();
    beginResetModel();
    rebuildVisible();
    endResetModel();
}

void MyTableModel::rebuildVisible()
{
    visible_.clear();
    visible_.reserve(items_.size());
    for (int i = 0; i < items_.size(); ++i) {
        if (passesFilter(items_.at(i)))
            visible_.append(i);
    }
}

bool MyTableModel::passesFilter(const ModelItem &item) const
{
    for (auto it = filters_.cbegin(); it != filters_.cend(); ++it) {
        const int column = it.key();
        const QSet<QString> &allowed = it.value();
        if (!allowed.contains(displayValue(column, item)))
            return false;
    }
    return true;
}

QStringList MyTableModel::distinctValues(int column) const
{
    QStringList result;
    QSet<QString> seen;
    for (const ModelItem &it : items_) {
        const QString v = displayValue(column, it);
        if (!seen.contains(v)) {
            seen.insert(v);
            result.append(v);
        }
    }
    return result;
}
