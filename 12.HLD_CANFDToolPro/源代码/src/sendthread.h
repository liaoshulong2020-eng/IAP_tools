#ifndef SENDTHREAD_H
#define SENDTHREAD_H
#include "modelitem.h"
#include <QByteArray>
#include <QList>
#include <QThread>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
class CanDevice;
class sendThread : public QThread
{
    Q_OBJECT
public:
    struct Config {
        int channel=0; bool fd=false,extended=false,remote=false,brs=false;
        quint32 id=0; QByteArray data; bool continuous=false; quint64 count=1;
        int intervalMs=0; bool idIncrement=false,dataIncrement=false; int sendType=0;
    };
    explicit sendThread(CanDevice *device, QObject *parent=nullptr);
    void start(const Config &config); void stop(); void setInterval(int intervalMs);
    bool isRunning() const { return running_.load(std::memory_order_relaxed); }
    QList<ModelItem> drain(int maximum);
signals:
    void endSend();
    void sendFailed(const QString &error);
protected:
    void run() override;
private:
    void enqueue(ModelItem &&item);
    CanDevice *device_=nullptr; Config config_;
    std::atomic_bool stopped_{false},running_{false};
    std::atomic_int intervalMs_{0}; std::atomic<quint64> intervalRevision_{0};
    std::mutex queueMutex_,waitMutex_; std::condition_variable waitWake_;
    std::deque<ModelItem> sent_; static constexpr std::size_t QueueLimit=20000;
};
#endif
