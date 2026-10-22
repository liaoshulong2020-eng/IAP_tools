#include "recvthread.h"
#include "candevice.h"
#include <QDateTime>

recvThread::recvThread(CanDevice *device, QObject *parent) : QThread(parent), device_(device) {}
void recvThread::stop() { stopped_.store(true, std::memory_order_release); }

void recvThread::run()
{
    constexpr quint32 BatchSize = 1024;
    QVector<ZCAN_Receive_Data> canFrames(BatchSize);
    QVector<ZCAN_ReceiveFD_Data> fdFrames(BatchSize);
    while (!stopped_.load(std::memory_order_acquire)) {
        bool receivedAny = false;
        for (int channel = 0; channel < 2; ++channel) {
            if (!device_ || !device_->isChannelStarted(channel)) continue;
            const quint32 canCount = device_->receiveCan(channel, canFrames.data(), BatchSize);
            receivedAny |= canCount != 0;
            QList<ModelItem> batch;
            batch.reserve(int(canCount));
            for (quint32 i = 0; i < canCount; ++i) {
                const auto &r = canFrames[int(i)]; ModelItem item;
                item.systemTime = QDateTime::currentMSecsSinceEpoch(); item.timestamp = r.timestamp;
                item.channel = channel; item.extended = IS_EFF(r.frame.can_id); item.remote = IS_RTR(r.frame.can_id);
                item.id = GET_ID(r.frame.can_id); item.dlc = r.frame.can_dlc;
                item.data = QByteArray(reinterpret_cast<const char *>(r.frame.data), r.frame.can_dlc);
                batch.push_back(std::move(item));
            }
            if (!batch.isEmpty()) emit recvData(batch);
            const quint32 fdCount = device_->receiveCanFd(channel, fdFrames.data(), BatchSize);
            receivedAny |= fdCount != 0;
            batch.clear();
            batch.reserve(int(fdCount));
            for (quint32 i = 0; i < fdCount; ++i) {
                const auto &r = fdFrames[int(i)]; ModelItem item;
                item.systemTime = QDateTime::currentMSecsSinceEpoch(); item.timestamp = r.timestamp;
                item.channel = channel; item.fd = true; item.extended = IS_EFF(r.frame.can_id);
                item.remote = IS_RTR(r.frame.can_id); item.brs = (r.frame.flags & CANFD_BRS) != 0;
                item.id = GET_ID(r.frame.can_id); item.dlc = r.frame.len;
                item.data = QByteArray(reinterpret_cast<const char *>(r.frame.data), r.frame.len);
                batch.push_back(std::move(item));
            }
            if (!batch.isEmpty()) emit recvData(batch);
        }
        if (!receivedAny) msleep(2);
    }
}
