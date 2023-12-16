/*
 * bsp.h
 *
 *  Created on: Jun 13, 2021
 *      Author: baram
 */

#ifndef SRC_BSP_BSP_H_
#define SRC_BSP_BSP_H_

#include "def.h"


#ifdef __cplusplus
 extern "C" {
#endif

#include "hardware/clocks.h"
#include "pico/stdlib.h"

#include "tusb.h"
#include "pico/stdio/driver.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"


bool bspInit(void);

void delay(uint32_t time_ms);
uint32_t millis(void);


void logPrintf(const char *fmt, ...);


#ifdef __cplusplus
 }
#endif

#endif /* SRC_BSP_BSP_H_ */