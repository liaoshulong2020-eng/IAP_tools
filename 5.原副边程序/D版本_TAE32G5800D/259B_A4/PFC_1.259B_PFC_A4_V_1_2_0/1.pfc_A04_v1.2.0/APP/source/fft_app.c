#include <math.h>
#define PI 3.14159265358979323846
#define N 1024  // 采样点数，必须是2的幂

typedef struct {
    float re;
    float im;
} Complex;

// 计算复数乘法
Complex complexMul(Complex a, Complex b) {
    Complex result;
    result.re = a.re * b.re - a.im * b.im;
    result.im = a.re * b.im + a.im * b.re;
    return result;
}

// 进行FFT变换
void fft(Complex *x, int n) {
    int i, j, k, m;
    int size = n;
    for (i = 0; i < n; i++) {
        j = 0;
        for (m = 1; m < n; m *= 2) {
            j = (j << 1) | (1 & (i / m));
        }
        if (j < i) {
            Complex temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }

    for (i = 1; i < n; i *= 2) {
        int step = i * 2;
        float angle = -PI / i;
        for (j = 0; j < i; j++) {
            Complex w;
            w.re = cos(angle * j);
            w.im = sin(angle * j);
            for (k = j; k < n; k += step) {
                m = k + i;
                Complex t = complexMul(w, x[m]);
                x[m].re = x[k].re - t.re;
                x[m].im = x[k].im - t.im;
                x[k].re += t.re;
                x[k].im += t.im;
            }
        }
    }
}
void ifft(Complex *X, int n) {
    // 逆FFT算法，将频域数据转换为时域
    for (int i = 0; i < n; i++) {
        X[i].re /= n;
        X[i].im /= n;
    }

    fft(X, n);  // 使用FFT函数进行逆变换，需要将FFT函数稍作修改

    // 交换实部和虚部
    for (int i = 0; i < n; i++) {
        double temp = X[i].re;
        X[i].re = X[i].im;
        X[i].im = temp;
    }
}

// 主函数，测试FFT
void fft_calcul() {
    Complex signal[N];
    for(int i = 0; i < N; i++) {
        signal[i].re = cos(2 * PI * i / N); // 测试信号：余弦波
        signal[i].im = 0;
    }

    fft(signal, N);  // 执行FFT

    for(int i = 0; i < N; i++) {
        printf("Frequency %d: Amplitude = %f\n", i, sqrt(signal[i].re * signal[i].re + signal[i].im * signal[i].im));
    }

    return 0;
}
