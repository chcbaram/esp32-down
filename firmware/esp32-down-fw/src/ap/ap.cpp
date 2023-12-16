#include "ap.h"
#include "module/esp32.h"


static void apCore1(void);






void apInit(void)
{
  cliOpen(HW_UART_CH_CLI, 115200);

  esp32Init();

  multicore_launch_core1(apCore1);  
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
