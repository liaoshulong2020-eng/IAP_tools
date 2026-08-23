#include "csvwriter.h"

#include <QFile>

#include <chrono>

CsvWriter::~CsvWriter()
{
    running_.store(false, std::memory_order_relaxed);
    cv_.notify_all();
    join();
}

bool CsvWriter::start(const QString &path, const QString &header, QString *error)
{
    // 停掉旧线程并等待其完成
    running_.store(false, std::memory_order_relaxed);
    cv_.notify_all();
    join();

    QFile probe(path);
    if (!probe.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error)
            *error = probe.errorString();
        return false;
    }
    probe.close();

    dropped_.store(0, std::memory_order_relaxed);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
    }

    running_.store(true, std::memory_order_relaxed);
    thread_ = std::thread(&CsvWriter::run, this, path, header);
    return true;
}

void CsvWriter::append(const QString &line)
{
    if (!running_.load(std::memory_order_relaxed))
        return;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.size() >= MaxQueue) {
            queue_.pop_front();  // 极端情况丢最旧，避免 OOM
            dropped_.fetch_add(1, std::memory_order_relaxed);
        }
        queue_.push_back(line);
    }
    cv_.notify_one();
}

void CsvWriter::stop()
{
    // 非阻塞：仅发停止信号，写线程在后台排空后自行退出
    running_.store(false, std::memory_order_relaxed);
    cv_.notify_all();
}

void CsvWriter::join()
{
    if (thread_.joinable())
        thread_.join();
}

void CsvWriter::run(const QString &path, const QString &header)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        running_.store(false, std::memory_order_relaxed);
        return;
    }
    file.write(header.toUtf8());

    for (;;) {
        std::deque<QString> batch;  // 局部变量，每轮自动清空，避免重复写
        bool shouldStop = false;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::milliseconds(100),
                         [this] { return !queue_.empty() || !running_.load(std::memory_order_relaxed); });
            batch.swap(queue_);
            shouldStop = !running_.load(std::memory_order_relaxed) && queue_.empty();
        }
        if (!batch.empty()) {
            for (const QString &line : batch)
                file.write(line.toUtf8());
            file.flush();  // 每批落盘一次
        }
        if (shouldStop)
            break;
    }
    file.flush();
    file.close();
}
