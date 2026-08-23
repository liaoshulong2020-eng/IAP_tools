#include "sendthread.h"
#include "candevice.h"
#include <QDateTime>
#include <algorithm>
#include <chrono>

sendThread::sendThread(CanDevice *device,QObject *parent):QThread(parent),device_(device){}
void sendThread::start(const Config &config){if(running_.exchange(true))return;if(QThread::isRunning())wait();config_=config;stopped_.store(false);intervalMs_.store(qMax(0,config.intervalMs));intervalRevision_.fetch_add(1);QThread::start();}
void sendThread::stop(){stopped_.store(true);running_.store(false);waitWake_.notify_all();}
void sendThread::setInterval(int value){intervalMs_.store(qMax(0,value));intervalRevision_.fetch_add(1);waitWake_.notify_all();}
void sendThread::enqueue(ModelItem &&item){std::lock_guard<std::mutex> lock(queueMutex_);if(sent_.size()>=QueueLimit)sent_.erase(sent_.begin(),sent_.begin()+std::ptrdiff_t(QueueLimit/4));sent_.push_back(std::move(item));}
QList<ModelItem> sendThread::drain(int maximum){QList<ModelItem> result;std::lock_guard<std::mutex> lock(queueMutex_);const int count=qMin(maximum,int(sent_.size()));result.reserve(count);for(int i=0;i<count;++i){result.push_back(std::move(sent_.front()));sent_.pop_front();}return result;}

void sendThread::run()
{
    Config cfg=config_; quint64 remaining=cfg.count; quint32 id=cfg.id; QByteArray data=cfg.data;
    const quint32 mask=cfg.extended?0x1FFFFFFFu:0x7FFu;
    while(!stopped_.load(std::memory_order_relaxed)&&(cfg.continuous||remaining>0)){
        QString error; bool ok=cfg.fd
            ?device_->sendCanFd(cfg.channel,id,cfg.extended,cfg.remote,cfg.brs,data,cfg.sendType,&error)
            :device_->sendCan(cfg.channel,id,cfg.extended,cfg.remote,data,cfg.sendType,&error);
        if(!ok){emit sendFailed(error);break;}
        ModelItem item;item.systemTime=QDateTime::currentMSecsSinceEpoch();item.channel=cfg.channel;
        item.transmit=true;item.fd=cfg.fd;item.extended=cfg.extended;item.remote=cfg.remote;item.brs=cfg.brs;
        item.id=id;item.dlc=data.size();item.data=data;enqueue(std::move(item));
        if(cfg.idIncrement)id=(id+1)&mask;
        if(cfg.dataIncrement&&!data.isEmpty())data.back()=char(uchar(data.back())+1);
        if(!cfg.continuous&&--remaining==0)break;
        const int interval=intervalMs_.load(std::memory_order_relaxed);
        if(interval>0){const quint64 revision=intervalRevision_.load(std::memory_order_relaxed);std::unique_lock<std::mutex> lock(waitMutex_);waitWake_.wait_for(lock,std::chrono::milliseconds(interval),[this,revision]{return stopped_.load(std::memory_order_relaxed)||intervalRevision_.load(std::memory_order_relaxed)!=revision;});}
        else QThread::yieldCurrentThread();
    }
    running_.store(false,std::memory_order_relaxed);emit endSend();
}
