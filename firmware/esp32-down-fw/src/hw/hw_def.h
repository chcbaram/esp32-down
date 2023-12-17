#ifndef SRC_HW_HW_DEF_H_
#define SRC_HW_HW_DEF_H_


#include "bsp.h"


#define _DEF_FIRMWATRE_VERSION    "V231111R1"
#define _DEF_BOARD_NAME           "ESP32-DOWN-FW"





#define _USE_HW_LED
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART
#define      HW_UART_MAX_CH         3
#define      HW_UART_CH_USB         _DEF_UART1
#define      HW_UART_CH_DEBUG       _DEF_UART2
#define      HW_UART_CH_ESP32       _DEF_UART3
#define      HW_UART_CH_CLI         HW_UART_CH_DEBUG

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    16
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    4
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_LOG
#define      HW_LOG_CH              HW_UART_CH_DEBUG
#define      HW_LOG_BOOT_BUF_MAX    1024
#define      HW_LOG_LIST_BUF_MAX    1024

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         8

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          1

#define _USE_HW_SPI
#define      HW_SPI_MAX_CH          1

#define _USE_HW_ST7789

#define _USE_HW_LCD
#define      HW_LCD_LVGL            1
#define      HW_LCD_WIDTH           240
#define      HW_LCD_HEIGHT          280



#define _PIN_GPIO_LCD_BL            4
#define _PIN_GPIO_LCD_DC            2
#define _PIN_GPIO_LCD_CS            5
#define _PIN_GPIO_LCD_RST           3


#endif /* SRC_HW_HW_DEF_H_ */