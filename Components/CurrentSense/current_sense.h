/*
 *  current_sense.h
 *
 *  Created on: 17 Aug 2026
 *  Author: jochman03
 */

#ifndef CURRENT_SENSE_H
#define CURRENT_SENSE_H

#include <stdint.h>

typedef struct current_sense_t {
    uint32_t scale_q16;
} current_sense_t;

#define CURRENT_SENSE_CALC_SCALE(vref_mv, adc_max, r_shunt_mohm, gain)                             \
    ((uint32_t)(((((uint64_t)(vref_mv) * 1000ULL) << 16) +                                         \
                 (((uint64_t)(adc_max) * (uint64_t)(gain) * (uint64_t)(r_shunt_mohm)) / 2U)) /     \
                ((uint64_t)(adc_max) * (uint64_t)(gain) * (uint64_t)(r_shunt_mohm))))

int32_t current_sense_to_ma(const current_sense_t* sense, uint16_t raw);

#endif /* CURRENT_SENSE_H */
