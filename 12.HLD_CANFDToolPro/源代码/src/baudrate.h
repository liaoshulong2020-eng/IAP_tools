#ifndef BAUDRATE_H
#define BAUDRATE_H

#include <QtGlobal>

// 波特率档位表（仲裁域 / 数据域）。索引在「参数设置」与「启动通道」之间保持一致，
// 配置里 paramABIT1/paramABIT2 存的就是这里的下标。
//
// 仲裁域按常用 CAN 波特率升序排列，下标 6 = 125k（默认）。
struct BaudEntry {
    quint32 value;
    const char *label;
};

static const BaudEntry ABIT_BAUDS[] = {
    { 5000, "5kbps" },       // 0
    { 10000, "10kbps" },     // 1
    { 20000, "20kbps" },     // 2
    { 40000, "40kbps" },     // 3
    { 50000, "50kbps" },     // 4
    { 100000, "100kbps" },   // 5
    { 125000, "125kbps" },   // 6  默认
    { 250000, "250kbps" },   // 7
    { 500000, "500kbps" },   // 8
    { 800000, "800kbps" },   // 9
    { 1000000, "1Mbps" },    // 10
};

static const BaudEntry DBIT_BAUDS[] = {
    { 5000000, "5Mbps" }, { 4000000, "4Mbps" }, { 2000000, "2Mbps" },
    { 1000000, "1Mbps" }, { 800000, "800kbps" }, { 500000, "500kbps" },
    { 250000, "250kbps" }, { 125000, "125kbps" }, { 100000, "100kbps" },
};

static constexpr int ABIT_BAUD_COUNT = int(sizeof(ABIT_BAUDS) / sizeof(ABIT_BAUDS[0]));
static constexpr int DBIT_BAUD_COUNT = int(sizeof(DBIT_BAUDS) / sizeof(DBIT_BAUDS[0]));

#endif // BAUDRATE_H
