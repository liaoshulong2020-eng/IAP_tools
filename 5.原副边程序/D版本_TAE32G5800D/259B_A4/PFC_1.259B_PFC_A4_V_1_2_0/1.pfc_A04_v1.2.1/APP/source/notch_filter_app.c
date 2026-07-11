/**
 * notch_filter.c
 * Ä¸Ïß 100Hz ¹¤ÆµÎÆ²¨ÏÝ²¨ÂË²¨Æ÷
 * Fs=10kHz, Fnotch=100Hz, BW=10Hz
 * µ¥¾«¶È¸¡µã°æ±¾
 */
#include "main.h"
#include "notch_filter_app.h"
void notch_init(volatile NotchState *s)
{
    s->w1 = 0.0f;
    s->w2 = 0.0f;
}

RAMCODE
float notch_tick(volatile NotchState *s, float x)
{
    float w = x         - NOTCH_A1 * s->w1 - NOTCH_A2 * s->w2;
    float y = NOTCH_B0 * w + NOTCH_B1 * s->w1 + NOTCH_B2 * s->w2;
    s->w2 = s->w1;
    s->w1 = w;
    return y;
}

void notch_reset(volatile NotchState *s)
{
    s->w1 = 0.0f;
    s->w2 = 0.0f;
}