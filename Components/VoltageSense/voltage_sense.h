/*
 *  voltage_sense.h
 *
 *  Created on: 17 Aug 2026
 *  Author: jochman03
 */

#ifndef VOLTAGE_SENSE_H
#define VOLTAGE_SENSE_H

#include <stdint.h>

typedef struct voltage_sense_t {
    uint32_t scale_q16;
} voltage_sense_t;

#define VOLTAGE_SENSE_CALC_SCALE(vref_mv, adc_max, r_high_ohm, r_low_ohm)                          \
    ((uint32_t)(((((uint64_t)(vref_mv) * ((uint64_t)(r_high_ohm) + (uint64_t)(r_low_ohm)))         \
                  << 16) +                                                                         \
                 (((uint64_t)(adc_max) * (uint64_t)(r_low_ohm)) / 2U)) /                           \
                ((uint64_t)(adc_max) * (uint64_t)(r_low_ohm))))

int32_t voltage_sense_to_mv(const voltage_sense_t* sense, uint16_t raw);

#endif /* VOLTAGE_SENSE_H */
