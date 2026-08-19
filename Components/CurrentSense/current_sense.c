#include "current_sense.h"

int32_t current_sense_to_ma(const current_sense_t* sense, uint16_t raw) {
    const uint32_t rounded = (1ULL << 15);
    return (int32_t)(((uint64_t)raw * (uint64_t)sense->scale_q16 + rounded) >> 16);
}
