//#include "main.h"
//#include <stdio.h>
//#include <math.h>
//#include "arm_math.h"  // 假设使用ARM CMSIS DSP库
//void analyzeHarmonics(float32_t *signal);
//extern void analyzeHarmonics(float32_t *signal);

//#define SAMPLES 512             // 采样点数，确保为2的幂
//#define SAMPLING_RATE 10000     // 采样率
//#define FFT_SIZE SAMPLES / 2    // FFT的大小
//#define BASE_FREQUENCY 90    // 略低于第二谐波频率
//#define MAX_FREQUENCY 260   // 略高于第五谐波频率

//float32_t input[SAMPLES];      // 存储采样数据的数组
//float32_t output[FFT_SIZE];    // FFT结果存储

//// 初始化FFT结构体
//arm_rfft_fast_instance_f32 S;
//float frequency;
//float magnitude;

//void analyzeHarmonics(float32_t *signal) {
//    // 初始化FFT
//    arm_rfft_fast_init_f32(&S, SAMPLES);

//    // 执行FFT
//    arm_rfft_fast_f32(&S, signal, output, 0);

//    // 分析FFT结果，识别谐波
//    for(int i = 1; i < FFT_SIZE; i++) {
//         frequency = i * SAMPLING_RATE / SAMPLES;
//         magnitude = sqrtf(output[i * 2] * output[i * 2] + output[i * 2 + 1] * output[i * 2 + 1]);

//        // 判断是否为谐波频率，这里以识别特定频率为例
//        if(frequency > BASE_FREQUENCY && frequency < MAX_FREQUENCY) {
//            printf("Harmonic found at %f Hz with magnitude %f\n", frequency, magnitude);
//        }
//    }
//}
#include <stdio.h>
#include <math.h>
#include "main.h"
// 定义所需的常量
#define SAMPLING_RATE  10000   // 采样率，单位Hz
#define TARGET_FREQ    100     // 目标频率，单位Hz
#define N              100     // 采样点数，与采样率和目标频率相关
#define M_PI 3.14159265358979323846

// 循环缓冲区和当前索引
uint32_t signal[N];
int currentIndex = 0;

// Goertzel算法函数
RAMCODE
float goertzel(int sampleRate, int targetFreq, int n) {
    float omega = (2.0 * M_PI * targetFreq) / sampleRate;
    float coeff = 2.0 * cos(omega);
    float q0 = 0, q1 = 0, q2 = 0;

    for(int i = 0; i < n; i++) {
        q0 = coeff * q1 - q2 + signal[i];
        q2 = q1;
        q1 = q0;
    }

    float real = (q1 - q2 * cos(omega));
    float imag = (q2 * sin(omega));
    return sqrt(real*real + imag*imag);
}
float magnitude;
// 数据采集函数
RAMCODE
void collectData(uint32_t newSample) {
    // 将新采样值加入循环缓冲区
    signal[currentIndex] = newSample;
    currentIndex = (currentIndex + 1) % N; // 更新索引，循环回到开始

    // 可以选择在缓冲区满时（每N个样本）执行Goertzel算法
    if (currentIndex == 0) {
        float magnitude = goertzel(SAMPLING_RATE, TARGET_FREQ, N);
        // 处理magnitude值，例如打印或者其他逻辑
        printf("Magnitude: %f\n", magnitude);
    }
}

