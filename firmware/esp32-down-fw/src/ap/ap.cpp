#include "ap.h"
#include "module/esp32.h"





void apInit(void)
{
  cliOpen(HW_UART_CH_CLI, 115200);

  esp32Init();
}

void apMain(void)
{
  uint32_t pre_time;


  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);      
    }

    cliMain();
    esp32Update();
  }
}

