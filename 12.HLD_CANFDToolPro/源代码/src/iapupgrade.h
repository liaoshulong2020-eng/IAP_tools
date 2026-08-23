#pragma once

#include <QByteArray>
#include <QString>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>

class IapUpgrade {
public:
    struct Options { QString firmwarePath; uint32_t canId{}; uint8_t targetAddress{}; };
    using SendFunction=std::function<bool(const QByteArray&)>;
    using LogFunction=std::function<void(const QString&)>;
    using ProgressFunction=std::function<void(int)>;
    using FinishedFunction=std::function<void(bool)>;

    IapUpgrade()=default;
    ~IapUpgrade();
    bool start(const Options &options,SendFunction send,LogFunction log,ProgressFunction progress,FinishedFunction finished);
    void cancel();
    void feed(uint32_t canId,const QByteArray &data);
    bool running() const{return running_.load();}

private:
    void run();
    QByteArray loadFirmware(const QString &path);
    QByteArray loadHex(const QString &path);
    QByteArray packet(uint16_t command,uint32_t address,uint16_t length,const QByteArray &payload) const;
    QByteArray sendAndWait(const QByteArray &request,int timeoutMs);
    bool enterIap();bool writeFlash(const QByteArray &firmware);bool writeChecksum(int size,uint32_t crc);bool exitIap();
    void saveResume(int index);int loadResume(const QByteArray &firmware,uint32_t crc);void clearResume();
    static uint16_t crc16(const QByteArray &data,int size=-1);static uint32_t crc32(const QByteArray &data);
    static uint16_t read16(const QByteArray &data,int offset);static uint32_t read32(const QByteArray &data,int offset);

    Options options_;SendFunction send_;LogFunction log_;ProgressFunction progress_;FinishedFunction finished_;uint32_t firmwareCrc_{};int firmwareSize_{};
    std::atomic_bool running_{false},cancelled_{false};std::thread thread_;std::mutex mutex_;std::condition_variable ackCondition_;QByteArray receiveBuffer_,ack_;bool waiting_{};int resumeIndex_{};
};
