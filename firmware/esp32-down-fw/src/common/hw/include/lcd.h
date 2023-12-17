/*
 * lcd.h
 *
 *  Created on: 2020. 8. 5.
 *      Author: Baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_LCD_H_
#define SRC_COMMON_HW_INCLUDE_LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lcd_def.h"



#ifdef _USE_HW_LCD

#if HW_LCD_LVGL == 1
#include "lvgl/lvgl.h"
#endif


#define LCD_WIDTH         HW_LCD_WIDTH
#define LCD_HEIGHT        HW_LCD_HEIGHT

#define GETR(c) (((uint16_t)(c)) >> 11)
#define GETG(c) (((c) & 0x07E0)>>5)
#define GETB(c) ((c) & 0x1F)
#define RGB2COLOR(r, g, b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))


typedef enum
{
  LCD_FONT_07x10,
  LCD_FONT_11x18,
  LCD_FONT_16x26,
  LCD_FONT_HAN,
  LCD_FONT_MAX
} LcdFont;


typedef enum
{
  LCD_ALIGN_H_LEFT    = (1<<0),
  LCD_ALIGN_H_CENTER  = (1<<1),
  LCD_ALIGN_H_RIGHT   = (1<<2),
  LCD_ALIGN_V_TOP     = (1<<3),
  LCD_ALIGN_V_CENTER  = (1<<4),
  LCD_ALIGN_V_BOTTOM  = (1<<5),
} LcdStringAlign;

typedef enum
{
  LCD_RESIZE_NEAREST,
  LCD_RESIZE_BILINEAR
} LcdResizeMode;


#ifdef __cplusplus
#define LVGL_IMG_DEF(var_name) extern "C" lvgl_img_t var_name;
#else
#define LVGL_IMG_DEF(var_name) extern lvgl_img_t var_name;
#endif


#ifdef HW_LCD_LVGL


typedef struct
{
  const lvgl_img_t *p_img;
  uint16_t color_tbl[256];
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} image_t;

typedef struct
{
  const lvgl_img_t *p_img;
  int16_t  x;
  int16_t  y;
  int16_t  w;
  int16_t  h;
  int16_t  stride_x;
  int16_t  stride_y;
  int16_t  cnt;
  uint32_t delay_ms;
} sprite_param_t;

typedef struct
{
  image_t image;
  sprite_param_t param;

  uint32_t pre_time;
  int16_t  cur_index;
  bool     is_init;
} sprite_t;

#endif


bool lcdInit(void);
bool lcdIsInit(void);
void lcdReset(void);

uint8_t lcdGetBackLight(void);
void    lcdSetBackLight(uint8_t value);

uint32_t lcdReadPixel(uint16_t x_pos, uint16_t y_pos);
void lcdClear(uint32_t rgb_code);
void lcdClearBuffer(uint32_t rgb_code);

bool lcdDrawAvailable(void);
bool lcdRequestDraw(void);
void lcdUpdateDraw(void);
void lcdSetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void lcdSendBuffer(uint8_t *p_data, uint32_t length, uint32_t timeout_ms);
void lcdDisplayOff(void);
void lcdDisplayOn(void);

void     lcdSetFps(int32_t fps);
uint32_t lcdGetFps(void);
uint32_t lcdGetFpsTime(void);
uint32_t lcdGetDrawTime(void);

int32_t lcdGetWidth(void);
int32_t lcdGetHeight(void);

uint16_t *lcdGetFrameBuffer(void);
uint16_t *lcdGetCurrentFrameBuffer(void);
void lcdSetDoubleBuffer(bool enable);

void lcdDrawPixel(int16_t x_pos, int16_t y_pos, uint32_t rgb_code);
void lcdDrawPixelMix(int16_t x_pos, int16_t y_pos, uint32_t rgb_code, uint8_t mix);
void lcdDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color);
void lcdDrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void lcdDrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void lcdDrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void lcdDrawFillScreen(uint16_t color);
void lcdDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void lcdDrawFillCircle(int32_t x0, int32_t y0, int32_t r, uint16_t color);
void lcdDrawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color);
void lcdDrawFillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color);
void lcdDrawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color);
void lcdDrawFillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color);
void lcdDrawString(int x, int y, uint16_t color, const char *str);
void lcdPrintf(int x, int y, uint16_t color,  const char *fmt, ...);
void lcdSetFont(LcdFont font);
LcdFont lcdGetFont(void);
void lcdPrintfResize(int x, int y, uint16_t color,  float ratio_h, const char *fmt, ...);
void lcdPrintfRect(int x, int y, int w, int h, uint16_t color, float ratio, uint16_t align, const char *fmt, ...);
void lcdSetResizeMode(LcdResizeMode mode);
uint32_t lcdGetStrWidth(const char *fmt, ...);

#ifdef HW_LCD_LVGL
image_t lcdCreateImage(lvgl_img_t *p_lvgl_img, int16_t x, int16_t y, int16_t w, int16_t h);
void lcdDrawImage(image_t *p_img, int16_t x, int16_t y);
void lcdDrawImageOffset(image_t *p_img, int16_t offset_x, int16_t offset_y, int16_t draw_x, int16_t draw_y);

bool lcdSpriteCreate(sprite_t *p_sprite);
void lcdSpriteDraw(sprite_t *p_sprite, int16_t x, int16_t y, uint16_t index);
void lcdSpriteDrawWrap(sprite_t *p_sprite, int16_t x, int16_t y, bool reset);

void lcdLogoOn(void);
void lcdLogoOff(void);
bool lcdLogoIsOn(void);
#endif


#endif /* _USE_HW_LCD */


#ifdef __cplusplus
}
#endif

#endif /* SRC_COMMON_HW_INCLUDE_LCD_H_ */
