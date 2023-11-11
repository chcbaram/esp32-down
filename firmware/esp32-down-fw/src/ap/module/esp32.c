#include "esp32.h"
#include "pico/bootrom.h"



static void esp32UpdateData(void);
static void esp32UpdateReset(void);
static void esp32UpdateBaud(void);
static void esp32Reset(bool boot);


static const uint8_t pin_esp_reset = 0;
static const uint8_t pin_esp_boot  = 1;
static uint32_t pre_time;
static bool is_log = true;
static bool is_req_baud = false;
static bool is_req_run  = false;
static bool is_req_boot = false;
static uint8_t  uart_down_ch  = HW_UART_CH_ESP32;
static uint8_t  usb_ch        = HW_UART_CH_USB;
static uint32_t uart_baud_req = 115200;






bool esp32Init(void)
{
  pre_time = millis();
  return true;
}


bool esp32Update(void)
{

  esp32UpdateBaud();
  esp32UpdateReset();
  esp32UpdateData();

  return true;
}

void esp32Reset(bool boot)
{
  if (boot == true)
  {
    uartFlush(uart_down_ch);
    uartFlush(usb_ch);

    gpioPinWrite(pin_esp_reset, _DEF_HIGH);
    gpioPinWrite(pin_esp_boot,_DEF_LOW);
    delay(1);
    gpioPinWrite(pin_esp_reset, _DEF_LOW);
    delay(1);
    gpioPinWrite(pin_esp_reset, _DEF_HIGH);
  }
  else
  {
    uartFlush(uart_down_ch);
    uartFlush(usb_ch);

    gpioPinWrite(pin_esp_reset, _DEF_HIGH);
    gpioPinWrite(pin_esp_boot,_DEF_HIGH);
    delay(1);
    gpioPinWrite(pin_esp_reset, _DEF_LOW);
    delay(1);
    gpioPinWrite(pin_esp_reset, _DEF_HIGH);
  }  
}

void esp32UpdatePacket(uint8_t data)
{
  const uint8_t  buf_len = 13;
  const uint8_t  pkt_i = buf_len - 2;
  static uint8_t buf[13] = {0, };

  for (int i=buf_len-1; i>=1; i--)
  {
    buf[i] = buf[i-1];
  }
  buf[0] = data;

  if (buf[buf_len-1] == 0xC0 && buf[buf_len-2] == 0x00)
  {
    data_t pkt_len;
    data_t pkt_data;


    pkt_len.u8Data[0] = buf[pkt_i-2];
    pkt_len.u8Data[1] = buf[pkt_i-3];

    pkt_data.u8Data[0] = buf[pkt_i-8];
    pkt_data.u8Data[1] = buf[pkt_i-9];
    pkt_data.u8Data[2] = buf[pkt_i-10];
    pkt_data.u8Data[3] = buf[pkt_i-11];

    logPrintf("%02X [%04d] 0x%08X\n", buf[pkt_i-1], pkt_len.u16D, pkt_data.u32D);

    // if (buf[9] == 0x02)
    //   logPrintf("FLASH_BEGIN\n");
    // if (buf[9] == 0x03)
    //   logPrintf("FLASH_DATA\n");
    // if (buf[9] == 0x04)
    //   logPrintf("FLASH_END\n");
  }
}

void esp32UpdateBaud(void)
{
  // USB에서 요청한 통신 속도와 ESP32와의 통신 속도가 다른 경우 
  // ESP32의 통신 속도를 변경한다. 
  //
  if (is_req_baud == true)
  {
    static uint32_t pre_time = 0;
    is_req_baud = false;
    if (uartGetBaud(uart_down_ch) != uart_baud_req)
    {
      if (uart_baud_req != 38400)
      {
        uartSetBaud(uart_down_ch, uart_baud_req);
        logPrintf("set baud : %d %d\n", uart_baud_req, millis()-pre_time);
        pre_time = millis();
      }
    }
  }  
}

void esp32UpdateReset(void)
{
  if (is_req_boot == true)
  {
    is_req_boot = false;
    esp32Reset(true);

    if (is_log)
    {
      logPrintf("esp32 retset-boot\n");
    }
  }

  if (is_req_run == true)
  {
    is_req_run = false;
    esp32Reset(false);

    if (is_log)
    {
      logPrintf("esp32 reset-run\n");
    }
  }
}

void esp32UpdateData(void)
{
  uint8_t data;


  while(uartAvailable(uart_down_ch) > 0)
  {
    data = uartRead(uart_down_ch);
    uartWrite(usb_ch, &data, 1);
  }

  while(uartAvailable(usb_ch) > 0)
  {
    data = uartRead(usb_ch);
    uartWrite(uart_down_ch, &data, 1);
    esp32UpdatePacket(data);
  }
}

void esp32UpdatePin(bool dtr, bool rts)
{
  static uint16_t dtr_rts = 0;
  static uint32_t pre_time = 0;


  dtr_rts <<= 8;
  dtr_rts  |= (dtr<<4) | (rts<<0);


  if (dtr_rts == 0x1000)
  {
    is_req_boot = true;
  }
  if (dtr_rts == 0x0100)
  {
    is_req_run = true;
  }

  if (is_log)
  {
    logPrintf("dtr : %d, rts : %d, %d ms\n", dtr, rts, millis()-pre_time);
    pre_time = millis();
  }
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  esp32UpdatePin(dtr, rts);
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
  is_req_baud = true;
  uart_baud_req = p_line_coding->bit_rate;

  if (p_line_coding->bit_rate == PICO_STDIO_USB_RESET_MAGIC_BAUD_RATE) 
  {
    #ifdef PICO_STDIO_USB_RESET_BOOTSEL_ACTIVITY_LED
    const uint gpio_mask = 1u << PICO_STDIO_USB_RESET_BOOTSEL_ACTIVITY_LED;
    #else
    const uint gpio_mask = 0u;
    #endif
    reset_usb_boot(gpio_mask, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK);  
  }
}

