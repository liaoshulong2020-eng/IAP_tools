#include <stdint.h>
#include "main.h"
#include "soft_notch_filter.h"
#include <string.h> // 为了使用memmove函数

// 滤波器系数 OK
const float b[] = {0.996812966709012093780017949029570445418 , -1.989691968038949765329448382544796913862  , 0.996812966709012093780017949029570445418  };
const float a[] = {1.0, -1.989691968038949765329448382544796913862  , 0.993625933418024187560035898059140890837  }; // a[0]总是1，因此在计算时可以省略

//const float b[] = {  0.999644883775386849933397570566739886999,-1.995344625888818113068623461003880947828,   0.999644883775386849933397570566739886999};
//const float a[] = {1.0, -1.995344625888818113068623461003880947828,   0.999289767550773699866795141133479773998 }; // a[0]总是1，因此在计算时可以省略

// 缓冲区定义
float x_buf[sizeof(b)/sizeof(b[0])] = {0}; // 输入缓冲区
float y_buf[sizeof(a)/sizeof(a[0]) - 1] = {0}; // 输出缓冲区，a[0]被省略

RAMCODE
float applyFilter(float input) {
    // 更新输入缓冲区
    memmove(&x_buf[1], x_buf, (sizeof(x_buf)/sizeof(x_buf[0]) - 1) * sizeof(float));
    x_buf[0] = input;

    // 应用滤波器
    float output = b[0] * x_buf[0];
    for (int i = 1; i < sizeof(b)/sizeof(b[0]); ++i) {
        output += b[i] * x_buf[i];
    }
    for (int i = 1; i < sizeof(a)/sizeof(a[0]); ++i) {
        output -= a[i] * y_buf[i - 1];
    }

    // 更新输出缓冲区
    memmove(&y_buf[1], y_buf, (sizeof(y_buf)/sizeof(y_buf[0]) - 1) * sizeof(float));
    y_buf[0] = output;

    return output;
}