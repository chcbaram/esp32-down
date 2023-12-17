#ifndef _LCD_DEF_H_
#define _LCD_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"


#define LCD_SWAP_COLOR    0



#if LCD_SWAP_COLOR == 0
enum class_color {
 white     = 0xFFFF,
 gray      = 0x8410,
 darkgray  = 0xAD55,
 black     = 0x0000,
 purple    = 0x8010,
 pink      = 0xFE19,
 red       = 0xF800,
 orange    = 0xFD20,
 brown     = 0xA145,
 beige     = 0xF7BB,
 yellow    = 0xFFE0,
 lightgreen= 0x9772,
 green     = 0x0400,
 darkblue  = 0x0011,
 blue      = 0x001F,
 lightblue = 0xAEDC,
};
#else
enum class_color {
 white     = 0xFFFF,
 gray      = 0x1084,
 darkgray  = 0x55AD,
 black     = 0x0000,
 purple    = 0x1080,
 pink      = 0x19FE,
 red       = 0x00F8,
 orange    = 0x20FD,
 brown     = 0x45A1,
 beige     = 0xBBF7,
 yellow    = 0xE0FF,
 lightgreen= 0x7297,
 green     = 0x0004,
 darkblue  = 0x1100,
 blue      = 0x1F00,
 lightblue = 0xDCAE,
};
#endif

typedef struct lcd_driver_t_ lcd_driver_t;

typedef struct lcd_driver_t_
{
  bool     (*init)(void);
  bool     (*reset)(void);
  void     (*setWindow)(int32_t x, int32_t y, int32_t w, int32_t h);
  uint16_t (*getWidth)(void);
  uint16_t (*getHeight)(void);
  bool     (*setCallBack)(void (*p_func)(void));
  bool     (*sendBuffer)(uint8_t *p_data, uint32_t length, uint32_t timeout_ms);

} lcd_driver_t;


#ifdef __cplusplus
}
#endif

#endif 