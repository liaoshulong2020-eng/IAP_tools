#ifndef CSVWRITER_H
#define CSVWRITER_H

#include <QString>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>

// 后台 CSV 写入器：独立线程 + 有界队列，逐帧写入、不阻塞 UI。
//
// stop() 只发停止信号（非阻塞），写线程会在后台把剩余队列排空后自行退出；
// join() 用于需要「确保已落盘」的时机（析构、重新 start、打开文件前）。
class CsvWriter
{
public:
    CsvWriter() = default;
    ~CsvWriter();

    CsvWriter(const CsvWriter &) = delete;
    CsvWriter &operator=(const CsvWriter &) = delete;

    bool start(const QString &path, const QString &header, QString *error = nullptr);
    void append(const QString &line);   // 非阻塞入队
    void stop();                        // 非阻塞：发停止信号
    void join();                        // 阻塞：等待写线程完成

    bool isRunning() const { return running_.load(std::memory_order_relaxed); }
    quint64 droppedCount() const { return dropped_.load(std::memory_order_relaxed); }

private:
    void run(const QString &path, const QString &header);

    std::atomic_bool running_{false};
    std::atomic_uint64_t dropped_{0};
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<QString> queue_;

    static constexpr std::size_t MaxQueue = 200000;  // 队列上限（防内存无限增长）
};

#endif // CSVWRITER_H
