/*
 *  faults.h
 *
 *  Created on: 19 Aug 2026
 *  Author: jochman03
 */

#ifndef FAULTS_H
#define FAULTS_H

typedef enum fault_type { FAULT_HARD, FAULT_HEAP, FAULT_STACK, FAULT_ASSERT } fault_type;

void faults_handle(fault_type type);
void faults_assert_handle(void);

#endif /* FAULTS_H */