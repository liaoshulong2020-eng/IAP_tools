#ifndef MODELITEM_H
#define MODELITEM_H

#include <QByteArray>
#include <QMetaType>
#include <QList>
#include <QString>
#include <cstdint>

// 一帧数据（收发表的一行）
struct ModelItem
{
    qint64    systemTime = 0;   // 系统时间（毫秒时间戳，接收时的墙上时钟）
    quint64   timestamp  = 0;   // 设备时间标识（us，基于设备启动时间）
    int       channel    = 0;   // 通道索引 0/1（显示为 1/2）
    bool      transmit   = false;// true=发送 false=接收
    bool      fd         = false;// true=CAN-FD false=CAN
    bool      extended   = false;// true=扩展帧 false=标准帧
    bool      remote     = false;// true=远程帧 false=数据帧
    bool      brs        = false;// CAN-FD 加速（BRS）
    quint32   id         = 0;    // 帧 ID（已去掉标志位）
    int       dlc        = 0;    // 数据长度
    QByteArray data;             // 数据（<=64）

    // 用于「合并列表数据」的等价键：同通道、同方向、同类型、同 ID 视为同一帧
    bool sameFrame(const ModelItem &o) const
    {
        return channel == o.channel && transmit == o.transmit && fd == o.fd
               && extended == o.extended && id == o.id;
    }
};

Q_DECLARE_METATYPE(ModelItem)
Q_DECLARE_METATYPE(QList<ModelItem>)

#endif // MODELITEM_H
