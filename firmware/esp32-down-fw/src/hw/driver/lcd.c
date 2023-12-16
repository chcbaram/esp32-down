#include "lcd.h"



#ifdef _USE_HW_LCD
#include "gpio.h"
#include "cli.h"
#include "lcd/st7789.h"
#include "lcd/lcd_fonts.h"
#include "hangul/han.h"

#ifdef _USE_HW_PWM
#include "pwm.h"
#endif


#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif




#define MAKECOL(r, g, b) ( ((r)<<11) | ((g)<<5) | (b))



#define LCD_OPT_DEF   __attribute__((optimize("O2")))
#define _PIN_DEF_BL_CTL       _PIN_GPIO_LCD_BL


typedef struct
{
  int16_t x;
  int16_t y;
} lcd_pixel_t;


static lcd_driver_t lcd;


static bool is_init = false;
static volatile bool is_tx_done = true;
static uint8_t backlight_value = 100;

static bool lcd_request_draw = false;

static volatile uint32_t fps_pre_time;
static volatile uint32_t fps_time;
static volatile uint32_t fps_count = 0;

static volatile int32_t draw_fps = -1;
static volatile uint32_t draw_pre_time = 0;
static LcdFont lcd_font = LCD_FONT_HAN;
static lcd_font_t *font_tbl[LCD_FONT_MAX] = { &font_07x10, &font_11x18, &font_16x26, &font_hangul};


static void lcdDrawLineBuffer(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, lcd_pixel_t *line);


#ifdef _USE_HW_CLI
static void cliLcd(cli_args_t *args);
#endif

void TransferDoneISR(void)
{
  fps_time = millis() - fps_pre_time;
  fps_pre_time = millis();

  if (fps_time > 0)
  {
    fps_count = 1000 / fps_time;
  }

  lcd_request_draw = false;
}






bool lcdInit(void)
{
  backlight_value = 100;


  is_init = st7789Init();
  st7789InitDriver(&lcd);

  uint32_t pre_time;  
  
  pre_time = millis();
  lcdClear(black);
  logPrintf("lcd \n");
  logPrintf("   draw : %d ms\n", millis()-pre_time);

  lcdSetBackLight(100);


  if (is_init != true)
  {
    return false;
  }

#ifdef _USE_HW_CLI
  cliAdd("lcd", cliLcd);
#endif

  return true;
}

bool lcdIsInit(void)
{
  return is_init;
}

void lcdReset(void)
{
  lcd.reset();
}

uint8_t lcdGetBackLight(void)
{
  return backlight_value;
}

void lcdSetBackLight(uint8_t value)
{
  value = constrain(value, 0, 100);

  if (value != backlight_value)
  {
    backlight_value = value;
  }

#ifdef _USE_HW_PWM
  pwmWrite(0, CMAP(value, 0, 100, 0, 255));
#else
  if (backlight_value > 0)
  {
    gpioPinWrite(_PIN_DEF_BL_CTL, _DEF_HIGH);
  }
  else
  {
    gpioPinWrite(_PIN_DEF_BL_CTL, _DEF_LOW);
  }
#endif
}

LCD_OPT_DEF void lcdDrawPixel(uint16_t x_pos, uint16_t y_pos, uint32_t rgb_code)
{
  if (x_pos < 0 || x_pos >= LCD_WIDTH) return;
  if (y_pos < 0 || y_pos >= LCD_HEIGHT) return;

  lcd.setWindow(x_pos, y_pos, x_pos, y_pos);
  lcd.sendBuffer((uint8_t *)&rgb_code, 1, 10);
}

LCD_OPT_DEF void lcdClear(uint32_t rgb_code)
{
  lcdDrawFillRect(0, 0, HW_LCD_WIDTH, HW_LCD_HEIGHT, rgb_code);
}

LCD_OPT_DEF void lcdDrawFillCircle(int32_t x0, int32_t y0, int32_t r, uint16_t color)
{
  int32_t  x  = 0;
  int32_t  dx = 1;
  int32_t  dy = r+r;
  int32_t  p  = -(r>>1);


  lcdDrawHLine(x0 - r, y0, dy+1, color);

  while(x<r)
  {

    if(p>=0) {
      dy-=2;
      p-=dy;
      r--;
    }

    dx+=2;
    p+=dx;

    x++;

    lcdDrawHLine(x0 - r, y0 + x, 2 * r+1, color);
    lcdDrawHLine(x0 - r, y0 - x, 2 * r+1, color);
    lcdDrawHLine(x0 - x, y0 + r, 2 * x+1, color);
    lcdDrawHLine(x0 - x, y0 - r, 2 * x+1, color);
  }
}

LCD_OPT_DEF void lcdDrawCircleHelper( int32_t x0, int32_t y0, int32_t r, uint8_t cornername, uint32_t color)
{
  int32_t f     = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * r;
  int32_t x     = 0;

  while (x < r)
  {
    if (f >= 0)
    {
      r--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4)
    {
      lcdDrawPixel(x0 + x, y0 + r, color);
      lcdDrawPixel(x0 + r, y0 + x, color);
    }
    if (cornername & 0x2)
    {
      lcdDrawPixel(x0 + x, y0 - r, color);
      lcdDrawPixel(x0 + r, y0 - x, color);
    }
    if (cornername & 0x8)
    {
      lcdDrawPixel(x0 - r, y0 + x, color);
      lcdDrawPixel(x0 - x, y0 + r, color);
    }
    if (cornername & 0x1)
    {
      lcdDrawPixel(x0 - r, y0 - x, color);
      lcdDrawPixel(x0 - x, y0 - r, color);
    }
  }
}

LCD_OPT_DEF void lcdDrawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  // smarter version
  lcdDrawHLine(x + r    , y        , w - r - r, color); // Top
  lcdDrawHLine(x + r    , y + h - 1, w - r - r, color); // Bottom
  lcdDrawVLine(x        , y + r    , h - r - r, color); // Left
  lcdDrawVLine(x + w - 1, y + r    , h - r - r, color); // Right

  // draw four corners
  lcdDrawCircleHelper(x + r        , y + r        , r, 1, color);
  lcdDrawCircleHelper(x + w - r - 1, y + r        , r, 2, color);
  lcdDrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
  lcdDrawCircleHelper(x + r        , y + h - r - 1, r, 8, color);
}

LCD_OPT_DEF void lcdDrawFillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint32_t color)
{
  int32_t f     = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -r - r;
  int32_t y     = 0;

  delta++;

  while (y < r)
  {
    if (f >= 0)
    {
      r--;
      ddF_y += 2;
      f     += ddF_y;
    }

    y++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1)
    {
      lcdDrawHLine(x0 - r, y0 + y, r + r + delta, color);
      lcdDrawHLine(x0 - y, y0 + r, y + y + delta, color);
    }
    if (cornername & 0x2)
    {
      lcdDrawHLine(x0 - r, y0 - y, r + r + delta, color); // 11995, 1090
      lcdDrawHLine(x0 - y, y0 - r, y + y + delta, color);
    }
  }
}

LCD_OPT_DEF void lcdDrawFillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  // smarter version
  lcdDrawFillRect(x, y + r, w, h - r - r, color);

  // draw four corners
  lcdDrawFillCircleHelper(x + r, y + h - r - 1, r, 1, w - r - r - 1, color);
  lcdDrawFillCircleHelper(x + r, y + r        , r, 2, w - r - r - 1, color);
}

LCD_OPT_DEF void lcdDrawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color)
{
  lcdDrawLine(x1, y1, x2, y2, color);
  lcdDrawLine(x1, y1, x3, y3, color);
  lcdDrawLine(x2, y2, x3, y3, color);
}

LCD_OPT_DEF void lcdDrawFillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t color)
{
  uint16_t max_line_size_12 = cmax(abs(x1-x2), abs(y1-y2));
  uint16_t max_line_size_13 = cmax(abs(x1-x3), abs(y1-y3));
  uint16_t max_line_size_23 = cmax(abs(x2-x3), abs(y2-y3));
  uint16_t max_line_size = max_line_size_12;
  uint16_t i = 0;

  if (max_line_size_13 > max_line_size)
  {
    max_line_size = max_line_size_13;
  }
  if (max_line_size_23 > max_line_size)
  {
    max_line_size = max_line_size_23;
  }

  lcd_pixel_t line[max_line_size];

  lcdDrawLineBuffer(x1, y1, x2, y2, color, line);
  for (i = 0; i < max_line_size_12; i++)
  {
    lcdDrawLine(x3, y3, line[i].x, line[i].y, color);
  }
  lcdDrawLineBuffer(x1, y1, x3, y3, color, line);
  for (i = 0; i < max_line_size_13; i++)
  {
    lcdDrawLine(x2, y2, line[i].x, line[i].y, color);
  }
  lcdDrawLineBuffer(x2, y2, x3, y3, color, line);
  for (i = 0; i < max_line_size_23; i++)
  {
    lcdDrawLine(x1, y1, line[i].x, line[i].y, color);
  }
}

bool lcdDrawAvailable(void)
{
  bool ret = false;
 
  if (draw_fps > 0)
  {
    if (!lcd_request_draw && millis()-draw_pre_time >= (1000/draw_fps))
    {
      draw_pre_time = millis();
      ret = true;
    } 
  }
  else
  {
    ret = !lcd_request_draw;
  }

  return ret;
}

bool lcdRequestDraw(void)
{
  if (is_init != true)
  {
    return false;
  }
  if (lcd_request_draw == true)
  {
    return false;
  }

  lcd.setWindow(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);

  lcd_request_draw = true;

  return true;
}

void lcdSetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  if (is_init != true)
  {
    return;
  }

  lcd.setWindow(x, y, w, h);
}

void lcdDisplayOff(void)
{
}

void lcdDisplayOn(void)
{
  lcdSetBackLight(lcdGetBackLight());
}

int32_t lcdGetWidth(void)
{
  return LCD_WIDTH;
}

int32_t lcdGetHeight(void)
{
  return LCD_HEIGHT;
}


void lcdDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;


  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++)
  {
    if (steep)
    {
      lcdDrawPixel(y0, x0, color);
    } else
    {
      lcdDrawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void lcdDrawLineBuffer(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, lcd_pixel_t *line)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;


  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++)
  {
    if (steep)
    {
      if (line != NULL)
      {
        line->x = y0;
        line->y = x0;
      }
      lcdDrawPixel(y0, x0, color);
    } else
    {
      if (line != NULL)
      {
        line->x = x0;
        line->y = y0;
      }
      lcdDrawPixel(x0, y0, color);
    }
    if (line != NULL)
    {
      line++;
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void lcdDrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  lcdDrawLine(x, y, x, y+h-1, color);
}

void lcdDrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  lcdDrawLine(x, y, x+w-1, y, color);
}

void lcdDrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  st7789FillRect(x, y, w, h, color);
}

void lcdDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  lcdDrawHLine(x, y, w, color);
  lcdDrawHLine(x, y+h-1, w, color);
  lcdDrawVLine(x, y, h, color);
  lcdDrawVLine(x+w-1, y, h, color);
}

void lcdDrawFillScreen(uint16_t color)
{
  lcdDrawFillRect(0, 0, HW_LCD_WIDTH, HW_LCD_HEIGHT, color);
}

void disHanFont(int x, int y, han_font_t *FontPtr, uint16_t textcolor)
{
  uint16_t    i, j, Loop;
  uint16_t  FontSize = FontPtr->Size_Char;
  uint16_t index_x;

  if (FontSize > 2)
  {
    FontSize = 2;
  }

  for ( i = 0 ; i < 16 ; i++ )        // 16 Lines per Font/Char
  {
    index_x = 0;
    for ( j = 0 ; j < FontSize ; j++ )      // 16 x 16 (2 Bytes)
    {
      uint8_t font_data;

      font_data = FontPtr->FontBuffer[i*FontSize +j];

      for( Loop=0; Loop<8; Loop++ )
      {
        if( (font_data<<Loop) & (0x80))
        {
          lcdDrawPixel(x + index_x, y + i, textcolor);
        }
        index_x++;
      }
    }
  }
}

void disEngFont(int x, int y, char ch, lcd_font_t *font, uint16_t textcolor)
{
  uint32_t i, b, j;


  // We gaan door het font
  for (i = 0; i < font->height; i++)
  {
    b = font->data[(ch - 32) * font->height + i];
    for (j = 0; j < font->width; j++)
    {
      if ((b << j) & 0x8000)
      {
        lcdDrawPixel(x + j, (y + i), textcolor);
      }
    }
  }
}

void lcdPrintf(int x, int y, uint16_t color,  const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i, x_Pre = x;
  han_font_t FontBuf;
  uint8_t font_width;
  uint8_t font_height;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  if (font_tbl[lcd_font]->data != NULL)
  {
    for( i=0; i<len; i+=Size_Char )
    {
      disEngFont(x, y, print_buffer[i], font_tbl[lcd_font], color);

      Size_Char = 1;
      font_width = font_tbl[lcd_font]->width;
      font_height = font_tbl[lcd_font]->height;
      x += font_width;

      if ((x+font_width) > HW_LCD_WIDTH)
      {
        x  = x_Pre;
        y += font_height;
      }
    }
  }
  else
  {
    for( i=0; i<len; i+=Size_Char )
    {
      hanFontLoad( &print_buffer[i], &FontBuf );

      disHanFont( x, y, &FontBuf, color);

      Size_Char = FontBuf.Size_Char;
      if (Size_Char >= 2)
      {
        font_width = 16;
        x += 2*8;
      }
      else
      {
        font_width = 8;
        x += 1*8;
      }

      if ((x+font_width) > HW_LCD_WIDTH)
      {
        x  = x_Pre;
        y += 16;
      }

      if( FontBuf.Code_Type == PHAN_END_CODE ) break;
    }
  }
}

#ifdef HW_LCD_LVGL
void lcdDrawImage(int16_t x, int16_t y, lcd_img_t *p_img)
{
  int16_t w;
  int16_t h;
  uint16_t *p_data;
  uint16_t pixel;

  w = p_img->header.w;
  h = p_img->header.h;
  p_data = (uint16_t *)p_img->data;

  for (int yi=0; yi<h; yi++)
  {
    for (int xi=0; xi<w; xi++)
    {
      pixel = p_data[w*yi + xi];
      if (pixel != green)
      {
        lcdDrawPixel(x+xi, y+yi, pixel);
      }
    }
  }
}
#endif

#ifdef _USE_HW_CLI
void cliLcd(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "test") == true)
  {
    uint32_t index = 0;

    while(cliKeepLoop())
    {
      lcdClear(black);
      lcdPrintf(25,16*0, green, "[LCD 테스트]");
      delay(100);
      
      lcdPrintf(25,16*1, green, "%d", index++);

      lcdDrawFillRect(  0,  70, 50, 50, red);
      lcdDrawFillRect( 50, 120, 50, 50, green);
      lcdDrawFillRect(100, 170, 50, 50, blue);

      lcdDrawFillRoundRect(100, 70, 50, 50, 10, blue);
      delay(100);
    }

    ret = true;
  }
  if (args->argc == 2 && args->isStr(0, "bl") == true)
  {
    uint8_t bl_value;

    bl_value = args->getData(1);

    lcdSetBackLight(bl_value);

    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("lcd test\n");
    cliPrintf("lcd bl 0~100\n");
  }
}
#endif



#endif