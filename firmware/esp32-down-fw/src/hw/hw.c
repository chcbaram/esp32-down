/*
 * hw.c
 *
 *  Created on: Jun 13, 2021
 *      Author: baram
 */

#include "hw.h"
#include "touch/cst816t.h"



bool hwInit(void)
{  
  bspInit();

  cliInit();
  logInit();
  uartInit();
  for (int i=0; i<HW_UART_MAX_CH; i++)
  {
    uartOpen(i, 115200);
  }
  logOpen(HW_LOG_CH, 115200);
  
  logPrintf("[ Firmware Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", _DEF_BOARD_NAME);
  logPrintf("Booting..Ver  \t\t: %s\r\n", _DEF_FIRMWATRE_VERSION);
  logPrintf("Clk sys  \t\t: %d\r\n", clock_get_hz(clk_sys));
  logPrintf("Clk peri \t\t: %d\r\n", clock_get_hz(clk_peri));
  logPrintf("Clk usb  \t\t: %d\r\n", clock_get_hz(clk_usb));
  logPrintf("Clk adc  \t\t: %d\r\n", clock_get_hz(clk_adc));
  logPrintf("Clk rtc  \t\t: %d\r\n", clock_get_hz(clk_rtc));
  logPrintf("Clk timer\t\t: %d\r\n", clock_get_hz(clk_ref));
  
  ledInit();
  gpioInit();
  i2cInit();
  spiInit();
  lcdInit();
  touchInit();

  logBoot(false);


  return true;
}