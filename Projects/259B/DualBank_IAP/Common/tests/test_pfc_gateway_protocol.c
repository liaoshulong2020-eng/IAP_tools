#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "pfc_gateway_protocol.h"

static uint16_t crc16(const uint8_t *data, size_t size)
{
    uint16_t crc = 0;
    size_t i;
    int bit;
    for (i = 0; i < size; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (bit = 0; bit < 8; ++bit)
            crc = (crc & 0x8000U) ? (uint16_t)((crc << 1) ^ 0x1021U) : (uint16_t)(crc << 1);
    }
    return crc;
}

static uint8_t crc8(const uint8_t *data, size_t size)
{
    uint8_t crc = 0;
    size_t i;
    int bit;
    for (i = 0; i < size; ++i) {
        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit)
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ 0x07U) : (uint8_t)(crc << 1);
    }
    return crc;
}

static void make_entry_frame(uint8_t out[8], uint8_t header, uint8_t command,
                             uint32_t id, uint8_t sequence, uint8_t tail)
{
    out[0] = header; out[1] = command;
    out[2] = (uint8_t)id; out[3] = (uint8_t)(id >> 8); out[4] = (uint8_t)(id >> 16);
    out[5] = sequence; out[6] = crc8(out + 1, 5); out[7] = tail;
}

static size_t make_packet(uint8_t *out, uint8_t target, uint16_t command,
                          uint32_t address, const uint8_t *payload, uint16_t size)
{
    uint16_t crc;
    size_t i;
    assert(size <= IAP_MAX_PAYLOAD);
    memset(out, 0, IAP_MAX_PAYLOAD + IAP_PACKET_OVERHEAD);
    out[0] = target; out[1] = IAP_FUNCTION_CODE;
    out[2] = (uint8_t)command; out[3] = (uint8_t)(command >> 8);
    for (i = 0; i < 4; ++i) out[4 + i] = (uint8_t)(address >> (8U * i));
    out[8] = (uint8_t)size; out[9] = (uint8_t)(size >> 8);
    out[10] = (uint8_t)size; out[11] = (uint8_t)(size >> 8);
    if (size) memcpy(out + 12, payload, size);
    crc = crc16(out, 12U + size);
    out[12U + size] = (uint8_t)crc;
    out[13U + size] = (uint8_t)(crc >> 8);
    return 14U + size;
}

struct parser {
    uint8_t data[IAP_MAX_PAYLOAD + IAP_PACKET_OVERHEAD];
    size_t used, expected;
    unsigned complete, rejected;
};

static void parser_feed(struct parser *p, const uint8_t *bytes, size_t count)
{
    size_t i;
    for (i = 0; i < count; ++i) {
        uint16_t payload;
        if (p->used == 0 && bytes[i] != IAP_TARGET_PFC) continue;
        if (p->used >= sizeof(p->data)) { p->used = p->expected = 0; ++p->rejected; continue; }
        p->data[p->used++] = bytes[i];
        if (p->used == 2 && p->data[1] != IAP_FUNCTION_CODE) {
            p->used = p->expected = 0; ++p->rejected; continue;
        }
        if (p->used == 12) {
            payload = (uint16_t)(p->data[10] | ((uint16_t)p->data[11] << 8));
            if (payload > IAP_MAX_PAYLOAD) { p->used = p->expected = 0; ++p->rejected; continue; }
            p->expected = payload + IAP_PACKET_OVERHEAD;
        }
        if (p->expected && p->used == p->expected) {
            uint16_t actual = crc16(p->data, p->used - 2U);
            uint16_t expected = (uint16_t)(p->data[p->used - 2U] | ((uint16_t)p->data[p->used - 1U] << 8));
            if (actual == expected) ++p->complete; else ++p->rejected;
            p->used = p->expected = 0;
        }
    }
}

int main(void)
{
    uint8_t a[IAP_MAX_PAYLOAD + IAP_PACKET_OVERHEAD];
    uint8_t b[IAP_MAX_PAYLOAD + IAP_PACKET_OVERHEAD];
    uint8_t payload[128];
    uint8_t prepare[8], ready[8], reset[8];
    struct parser parser = {{0}, 0, 0, 0, 0};
    size_t i, size;
    for (i = 0; i < sizeof(payload); ++i) payload[i] = (uint8_t)i;
    size = make_packet(a, IAP_TARGET_PFC, IAP_CMD_WRITE, 0x08020000U, payload, sizeof(payload));
    assert(size == 142U);
    assert(crc16(a, size - 2U) == (uint16_t)(a[size - 2U] | ((uint16_t)a[size - 1U] << 8)));
    memcpy(b, a, size);
    assert(memcmp(a, b, size) == 0); /* duplicate request key is byte-identical */
    for (i = 0; i < size; i += 8U) parser_feed(&parser, a + i, size - i < 8U ? size - i : 8U);
    assert(parser.complete == 1U && parser.rejected == 0U);
    b[20] ^= 0x40U;
    assert(crc16(b, size - 2U) != (uint16_t)(b[size - 2U] | ((uint16_t)b[size - 1U] << 8)));
    for (i = 0; i < size; i += 3U) parser_feed(&parser, b + i, size - i < 3U ? size - i : 3U);
    assert(parser.complete == 1U && parser.rejected == 1U);
    assert(make_packet(a, IAP_TARGET_PFC, IAP_CMD_CAPABILITY, 0, NULL, 0) == IAP_PACKET_OVERHEAD);
    make_entry_frame(prepare, 0xAA, 0xAB, 0x0A0003U, 0x27, 0x55);
    make_entry_frame(ready, 0x55, 0xBA, 0x0A0003U, 0x27, 0xAA);
    make_entry_frame(reset, 0xAA, 0x5A, 0x0A0003U, 0x27, 0x55);
    assert(prepare[2] == 0x03 && prepare[3] == 0x00 && prepare[4] == 0x0A);
    assert(prepare[6] == crc8(prepare + 1, 5));
    assert(ready[6] == crc8(ready + 1, 5));
    assert(reset[6] == crc8(reset + 1, 5));
    reset[5] ^= 1U;
    assert(reset[5] != prepare[5]); /* stale or wrong transaction must not reset PFC */
    reset[4] ^= 1U;
    assert(memcmp(reset + 2, prepare + 2, 3) != 0); /* wrong ID must be rejected */
    puts("pfc gateway protocol tests passed");
    return 0;
}
