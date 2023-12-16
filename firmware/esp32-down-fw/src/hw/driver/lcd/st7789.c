/*
 * st7789.c
 *
 *  Created on: 2022. 9. 9.
 *      Author: baram
 */


#include "spi.h"
#include "gpio.h"
#include "cli.h"
#include "lcd/st7789.h"
#include "lcd/st7789_regs.h"


#ifdef _USE_HW_ST7789



#define _PIN_DEF_BL     _PIN_GPIO_LCD_BL
#define _PIN_DEF_DC     _PIN_GPIO_LCD_DC
#define _PIN_DEF_CS     _PIN_GPIO_LCD_CS
#define _PIN_DEF_RST    _PIN_GPIO_LCD_RST


#define MADCTL_MY       0x80
#define MADCTL_MX       0x40
#define MADCTL_MV       0x20
#define MADCTL_ML       0x10
#define MADCTL_RGB      0x00
#define MADCTL_BGR      0x08
#define MADCTL_MH       0x04




static void writecommand(uint8_t c);
static void writedata(uint8_t d);
static void st7789InitRegs(void);
static void st7789SetRotation(uint8_t m);
static bool st7789Reset(void);
static bool st7789SendBuffer(uint8_t *p_data, uint32_t length, uint32_t timeout_ms);


static uint8_t   spi_ch = _DEF_SPI1;

static const int32_t _width  = HW_LCD_WIDTH;
static const int32_t _height = HW_LCD_HEIGHT;
static void (*frameCallBack)(void) = NULL;
volatile static bool  is_write_frame = false;


const uint32_t colstart = 0;
const uint32_t rowstart = 20;



#ifdef _USE_HW_CLI
static void cliCmd(cli_args_t *args);
#endif

static void transferDoneISR(void)
{
  if (is_write_frame == true)
  {
    is_write_frame = false;    

    if (frameCallBack != NULL)
    {
      frameCallBack();
    }
  }
}





bool st7789Init(void)
{
  bool ret = true;

  ret = st7789Reset();

#ifdef _USE_HW_CLI
  cliAdd("st7789", cliCmd);
#endif

  return ret;
}

bool st7789InitDriver(lcd_driver_t *p_driver)
{
  p_driver->init = st7789Init;
  p_driver->reset = st7789Reset;
  p_driver->setWindow = st7789SetWindow;
  p_driver->getWidth = st7789GetWidth;
  p_driver->getHeight = st7789GetHeight;
  p_driver->setCallBack = st7789SetCallBack;
  p_driver->sendBuffer = st7789SendBuffer;
  return true;
}

bool st7789Reset(void)
{
  spiBegin(spi_ch);
  spiSetDataMode(spi_ch, SPI_MODE0);
  spiSetBaudRate(spi_ch, 32*1000000);
  spiAttachTxInterrupt(spi_ch, transferDoneISR);


  gpioPinWrite(_PIN_DEF_BL , _DEF_LOW);
  gpioPinWrite(_PIN_DEF_DC , _DEF_HIGH);
  gpioPinWrite(_PIN_DEF_CS , _DEF_HIGH);
  gpioPinWrite(_PIN_DEF_RST, _DEF_LOW);
  delay(10);
  gpioPinWrite(_PIN_DEF_RST, _DEF_HIGH);

  st7789InitRegs();

  st7789SetRotation(2);
  st7789FillRect(0, 0, HW_LCD_WIDTH, HW_LCD_HEIGHT, black);  
  return true;
}

uint16_t st7789GetWidth(void)
{
  return _width;
}

uint16_t st7789GetHeight(void)
{
  return _height;
}

void writecommand(uint8_t c)
{
  gpioPinWrite(_PIN_DEF_DC, _DEF_LOW);
  gpioPinWrite(_PIN_DEF_CS, _DEF_LOW);

  spiTransfer8(spi_ch, c);

  gpioPinWrite(_PIN_DEF_CS, _DEF_HIGH);
}

void writedata(uint8_t d)
{
  gpioPinWrite(_PIN_DEF_DC, _DEF_HIGH);
  gpioPinWrite(_PIN_DEF_CS, _DEF_LOW);

  spiTransfer8(spi_ch, d);

  gpioPinWrite(_PIN_DEF_CS, _DEF_HIGH);
}

void st7789InitRegs(void)
{
  writecommand(ST7789_SWRESET); //  1: Software reset, 0 args, w/delay
  delay(10);

  writecommand(ST7789_SLPOUT);  //  2: Out of sleep mode, 0 args, w/delay
  delay(10);

  writecommand(ST7789_INVON);  // 13: Don't invert display, no args, no delay

  writecommand(ST7789_MADCTL);  // 14: Memory access control (directions), 1 arg:
  writedata(0x08);              //     row addr/col addr, bottom to top refresh

  writecommand(ST7789_COLMOD);  // 15: set color mode, 1 arg, no delay:
  writedata(0x05);              //     16-bit color


  writecommand(ST7789_CASET);   //  1: Column addr set, 4 args, no delay:
  writedata(0x00);
  writedata(0x00);              //     XSTART = 0
  writedata((uint8_t)((HW_LCD_WIDTH-1)>>8));
  writedata((uint8_t)((HW_LCD_WIDTH-1)>>0));    //     XEND = 

  writecommand(ST7789_RASET);   //  2: Row addr set, 4 args, no delay:
  writedata(0x00);
  writedata(0x00);              //     XSTART = 0
  writedata((uint8_t)((HW_LCD_HEIGHT-1)>>8));
  writedata((uint8_t)((HW_LCD_HEIGHT-1)>>0));   //     XEND = 


  writecommand(ST7789_NORON);   //  3: Normal display on, no args, w/delay
  delay(10);
  writecommand(ST7789_DISPON);  //  4: Main screen turn on, no args w/delay
  delay(10);
}

void st7789SetRotation(uint8_t mode)
{
  writecommand(ST7789_MADCTL);

  switch (mode)
  {
   case 0:
     writedata(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
     break;

   case 1:
     writedata(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
     break;

  case 2:
    writedata(MADCTL_RGB);
    break;

   case 3:
     writedata(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
     break;
  }
}

void st7789SetWindow(int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
  spiSetBitWidth(spi_ch, 8);

  writecommand(ST7789_CASET);   // Column addr set
  writedata((x0+colstart)>>8);
  writedata((x0+colstart)>>0);  // XSTART
  writedata((x1+colstart)>>8);
  writedata((x1+colstart)>>0);  // XEND

  writecommand(ST7789_RASET);   // Row addr set
  writedata((y0+rowstart)>>8);
  writedata((y0+rowstart)>>0);  // YSTART
  writedata((y1+rowstart)>>8);
  writedata((y1+rowstart)>>0);  // YEND

  writecommand(ST7789_RAMWR);   // write to RAM
}

void st7789FillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
  uint16_t line_buf[w];

  // Clipping
  if ((x >= _width) || (y >= _height)) return;

  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }

  if ((x + w) > _width)  w = _width  - x;
  if ((y + h) > _height) h = _height - y;

  if ((w < 1) || (h < 1)) return;


  st7789SetWindow(x, y, x + w - 1, y + h - 1);
  spiSetBitWidth(spi_ch, 16);

  gpioPinWrite(_PIN_DEF_DC, _DEF_HIGH);
  gpioPinWrite(_PIN_DEF_CS, _DEF_LOW);

  for (int i=0; i<w; i++)
  {
    line_buf[i] = color;
  }
  for (int i=0; i<h; i++)
  {
    if (spiDmaTxTransfer(_DEF_SPI1, (void *)line_buf, w, 10) != true)
    {
      break;
    }
  }
  gpioPinWrite(_PIN_DEF_CS, _DEF_HIGH);
  spiSetBitWidth(spi_ch, 8);
}

bool st7789SendBuffer(uint8_t *p_data, uint32_t length, uint32_t timeout_ms)
{
  is_write_frame = true;

  spiSetBitWidth(spi_ch, 16);

  gpioPinWrite(_PIN_DEF_DC, _DEF_HIGH);
  gpioPinWrite(_PIN_DEF_CS, _DEF_LOW);

  spiDmaTxTransfer(_DEF_SPI1, (void *)p_data, length, 0);
  return true;
}

bool st7789SetCallBack(void (*p_func)(void))
{
  frameCallBack = p_func;

  return true;
}


#ifdef _USE_HW_CLI

static void frameISR(void)
{
  static int i = 0;

  cliPrintf("done %d\n", i++);
}

void cliCmd(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("Width  : %d\n", _width);
    cliPrintf("Heigth : %d\n", _height);
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "test"))
  {
    uint32_t pre_time;
    uint32_t pre_time_draw;
    uint16_t color_tbl[3] = {red, green, blue};
    uint8_t color_index = 0;
    void (*cb_func_pre)(void);

    cb_func_pre = frameCallBack;

    st7789SetCallBack(frameISR);

    pre_time = millis();
    while(cliKeepLoop())
    {
      if (millis()-pre_time >= 500)
      {
        pre_time = millis();

        if (color_index == 0) cliPrintf("draw red\n");
        if (color_index == 1) cliPrintf("draw green\n");
        if (color_index == 2) cliPrintf("draw blue\n");
        
        pre_time_draw = millis();
        st7789FillRect(0, 0, HW_LCD_WIDTH, HW_LCD_HEIGHT, color_tbl[color_index]);
        color_index = (color_index + 1)%3;

        cliPrintf("draw time : %d ms\n", millis()-pre_time_draw);
      }
      delay(1);
    }

    st7789SetCallBack(cb_func_pre);
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("st7789 info\n");
    cliPrintf("st7789 test\n");
  }
}


#endif

#endif