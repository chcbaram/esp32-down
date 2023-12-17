#include "ap.h"
#include "module/esp32.h"


static void apCore1(void);



LVGL_IMG_DEF(run_img_v);


image_t img_santa;
sprite_t santa_sprite;

void apInit(void)
{
  cliOpen(HW_UART_CH_CLI, 115200);

  esp32Init();

  multicore_launch_core1(apCore1);  

  logPrintf("cnt : %d, %d\n", run_img_v.header.h, run_img_v.header.w);

  img_santa = lcdCreateImage(&run_img_v, 0, 0, 240, 240);

  // img_santa = lcdCreateSpriteImage(&run_img_v, 0, 0, 240, 240, 11, );

  santa_sprite.param.p_img = &run_img_v;
  santa_sprite.param.x = 0;
  santa_sprite.param.y = 0;
  santa_sprite.param.w = 200;
  santa_sprite.param.h = 256;
  santa_sprite.param.stride_x = 0;
  santa_sprite.param.stride_y = 256;
  santa_sprite.param.cnt = run_img_v.header.h/256;
  santa_sprite.param.delay_ms = 60;
  lcdSpriteCreate(&santa_sprite);

  logPrintf("cnt : %d, %d\n", run_img_v.header.h, santa_sprite.param.cnt);
}

void apMain(void)
{
  uint32_t pre_time;
  int16_t pos_x = -200;

  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);      
    } 
    cliMain();     

    if (lcdDrawAvailable() == true)
    {
      lcdClearBuffer(black);

      lcdSpriteDrawWrap(&santa_sprite, pos_x, 50, false);
      pos_x = pos_x + 10;
      if (pos_x >= lcdGetWidth())
      {
        pos_x = -200;
      }

      if (pos_x >= -100 && pos_x < 50)
      {
        lcdPrintfRect(0, 20, LCD_WIDTH, 32, white, 32, 
                      LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, 
                      "안녕!");
      }
      if (pos_x > 50 && pos_x < lcdGetWidth()-60)
      {
        lcdPrintfRect(0, 20, LCD_WIDTH, 32, white, 32, 
                      LCD_ALIGN_H_CENTER|LCD_ALIGN_V_CENTER, 
                      "친구들~");
      }

      lcdRequestDraw();
    }
  }
}

void apCore1(void)
{
  while(1)
  {
    esp32Update();      
    delay(2);
  }
}
