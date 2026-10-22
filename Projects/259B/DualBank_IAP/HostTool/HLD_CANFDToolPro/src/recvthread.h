#ifndef RECVTHREAD_H
#define RECVTHREAD_H
#include "modelitem.h"
#include "inc/controlcanfd.h"
#include <QList>
#include <QThread>
#include <atomic>
#include <deque>
#include <mutex>
class CanDevice;
class recvThread : public QThread
{
    Q_OBJECT
public:
    explicit recvThread(CanDevice *device, QObject *parent = nullptr);
    void stop();
signals:
    void recvData(const QList<ModelItem> &items);
protected:
    void run() override;
private:
    CanDevice *device_ = nullptr;
    std::atomic_bool stopped_{false};
};
#endif
