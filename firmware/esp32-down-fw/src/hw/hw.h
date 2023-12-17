#ifndef SRC_HW_HW_H_
#define SRC_HW_HW_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "hw_def.h"



#include "led.h"
#include "uart.h"
#include "cli.h"
#include "log.h"
#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "lcd.h"
#include "touch.h"


bool hwInit(void);

#ifdef __cplusplus
}
#endif

#endif 