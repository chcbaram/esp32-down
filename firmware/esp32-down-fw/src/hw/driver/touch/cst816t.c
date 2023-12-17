#include "touch/cst816t.h"


#ifdef _USE_HW_CST816T
#include "i2c.h"
#include "gpio.h"
#include "cli.h"


#define REG_GESTURE_ID      0x01
#define REG_FINGER_NUM      0x02
#define REG_XPOS_H          0x03
#define REG_XPOS_L          0x04
#define REG_YPOS_H          0x05
#define REG_YPOS_L          0x06
#define REG_CHIP_ID         0xA7
#define REG_PROJ_ID         0xA8
#define REG_FW_VERSION      0xA9
#define REG_FACTORY_ID      0xAA
#define REG_SLEEP_MODE      0xE5
#define REG_IRQ_CTL         0xFA
#define REG_LONG_PRESS_TICK 0xEB
#define REG_MOTION_MASK     0xEC
#define REG_DIS_AUTOSLEEP   0xFE

#define CHIPID_CST716       0x20
#define CHIPID_CST816S      0xB4
#define CHIPID_CST816T      0xB5
#define CHIPID_CST816D      0xB6


#define MOTION_MASK_CONTINUOUS_LEFT_RIGHT 0b100
#define MOTION_MASK_CONTINUOUS_UP_DOWN    0b010
#define MOTION_MASK_DOUBLE_CLICK          0b001

#define IRQ_EN_TOUCH      0x40
#define IRQ_EN_CHANGE     0x20
#define IRQ_EN_MOTION     0x10
#define IRQ_EN_LONGPRESS  0x01



#ifdef _USE_HW_RTOS
#define lock()      xSemaphoreTake(mutex_lock, portMAX_DELAY);
#define unLock()    xSemaphoreGive(mutex_lock);
#else
#define lock()      
#define unLock()    
#endif

#define CST816T_TOUCH_WIDTH    240
#define CST816T_TOUCH_HEIGTH   280



static void cliCmd(cli_args_t *args);
static bool readRegs(uint16_t reg_addr, void *p_data, uint32_t length);
static bool writeRegs(uint16_t reg_addr, void *p_data, uint32_t length);
static bool cst816tInitRegs(void);

static uint8_t i2c_ch   = _DEF_I2C1;
static uint8_t i2c_addr = 0x15; 
static bool is_init = false;
static bool is_detected = false;
static volatile bool is_pressed = false;
static uint8_t i2c_retry = 0;
#ifdef _USE_HW_RTOS
static SemaphoreHandle_t mutex_lock = NULL;
#endif

static void gpioISR(uint gpio, uint32_t events) 
{
  if (gpio == 15)
  {
    is_pressed = true;
  }
}





bool cst816tInit(void)
{
  bool ret = false;

  #ifdef _USE_HW_RTOS
  if (mutex_lock == NULL)
  {
    mutex_lock = xSemaphoreCreateMutex();
  }
  #endif

  if (i2cIsBegin(i2c_ch) == true)
    ret = true;
  else
    ret = i2cBegin(i2c_ch, 100);


  gpioPinWrite(_PIN_GPIO_LCD_TP_RST, _DEF_LOW);
  delay(10);
  gpioPinWrite(_PIN_GPIO_LCD_TP_RST, _DEF_HIGH);
  delay(30);

  if (ret == true && i2cIsDeviceReady(i2c_ch, i2c_addr))
  {
    bool i2c_ret;
    uint8_t buf[4];

    i2c_ret = readRegs(REG_CHIP_ID, buf, 4);
    if (i2c_ret && buf[0] == CHIPID_CST816T)
    {
      is_detected = true;
    }
    else
    {
      logPrintf("[  ] Not CST826T\n");
    }
  }
  else
  {
    ret = false;
  }

  if (is_detected == true)
  {
    ret = cst816tInitRegs();

    gpio_set_irq_enabled_with_callback(15, GPIO_IRQ_EDGE_FALL, true, &gpioISR);
  }

  is_init = ret;
  
  logPrintf("[%s] cst816tInit()\n", ret ? "OK":"NG");

  cliAdd("cst816t", cliCmd);

  return ret;
}

bool cst816tInitRegs(void)
{
  uint8_t reg;


  // Disable Auto Sleep
  reg = 0xFF;
  writeRegs(REG_DIS_AUTOSLEEP, &reg, 1);

  return true;
}

bool readRegs(uint16_t reg_addr, void *p_data, uint32_t length)
{
  bool ret;

  lock();
  ret = i2cReadBytes(i2c_ch, i2c_addr, reg_addr, p_data, length, 10);
  unLock();

  return ret;
}

bool writeRegs(uint16_t reg_addr, void *p_data, uint32_t length)
{
  bool ret;

  lock();
  ret = i2cWriteBytes(i2c_ch, i2c_addr, reg_addr, p_data, length, 10);
  unLock();

  return ret;
}

uint16_t cst816tGetWidth(void)
{
  return CST816T_TOUCH_WIDTH;
}

uint16_t cst816tGetHeight(void)
{
  return CST816T_TOUCH_HEIGTH;
}

bool cst816tGetInfo(cst816t_info_t *p_info)
{
  bool ret = false;
  uint8_t buf[6] = {0};
  uint8_t touch_cnt = 0;


  p_info->count = 0;

  if (is_init == false)
  {
    return false;
  }


  if (is_pressed == false)
  {
    return true;
  }
  is_pressed = false;
  
  touch_cnt = 1; 
  p_info->count = touch_cnt;
  for (int i=0; i<touch_cnt; i++)
  {
    ret = readRegs(REG_GESTURE_ID, buf, 6);
    if (ret == true)
    {
      p_info->point[i].id     = buf[0];
      p_info->point[i].num    = buf[1];
      p_info->point[i].event  = 0;

      p_info->point[i].x = ((buf[2]&0x0F)<<8) | (buf[3]<<0);
      p_info->point[i].y = ((buf[4]&0x0F)<<8) | (buf[5]<<0); 
    }
    else
    {
      p_info->count = 0;
      if (i2c_retry < 3)
      {
        ret = i2cRecovery(i2c_ch);
        if (ret)
        {
          i2c_retry = 0;
        }
        else
        {
          i2c_retry++;
        }
      }
      break;
    }
  }

  return ret;
}

const char *cst816tGetGestureStr(uint8_t id)
{
  const char *p_str = NULL;

  switch (id) 
  {
    case GESTURE_NONE:
      p_str = "NONE";
      break;
    case GESTURE_SWIPE_DOWN:
      p_str = "SWIPE DOWN";
      break;
    case GESTURE_SWIPE_UP:
      p_str = "SWIPE UP";
      break;
    case GESTURE_SWIPE_LEFT:
      p_str = "SWIPE LEFT";
      break;
    case GESTURE_SWIPE_RIGHT:
      p_str = "SWIPE RIGHT";
      break;
    case GESTURE_SINGLE_CLICK:
      p_str = "SINGLE CLICK";
      break;
    case GESTURE_DOUBLE_CLICK:
      p_str = "DOUBLE CLICK";
      break;
    case GESTURE_LONG_PRESS:
      p_str = "LONG PRESS";
      break;
    default:
      p_str = "UNKNOWN 0x";
      break;
  }  

  return p_str;
}

void cliCmd(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("is_init     : %s\n", is_init ? "True" : "False");
    cliPrintf("is_detected : %s\n", is_detected ? "True" : "False");
    cliPrintf("is_addr     : 0x%02X\n", i2c_addr);
    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "read"))
  {
    uint16_t addr;
    uint16_t len;
    uint8_t data;

    addr = args->getData(1);
    len  = args->getData(2);

    for (int i=0; i<len; i++)
    {
      if (readRegs(addr + i, &data, 1) == true)
      {
        cliPrintf("0x%04x : 0x%02X\n", addr + i, data);
      }
      else
      {
        cliPrintf("readRegs() Fail\n");
        break;
      }
    }

    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "write"))
  {
    uint16_t addr;
    uint8_t  data;

    addr = args->getData(1);
    data = args->getData(2);


    if (writeRegs(addr, &data, 1) == true)
    {
      cliPrintf("0x%02x : 0x%02X\n", addr, data);
    }
    else
    {
      cliPrintf("writeRegs() Fail\n");
    }

    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "get") && args->isStr(1, "info"))
  {
    cst816t_info_t info;
    uint32_t pre_time;
    uint32_t exe_time;

    cliPrintf(" \n");
    cliPrintf("ready\n");

    while(cliKeepLoop())
    {
      pre_time = millis();
      if (cst816tGetInfo(&info) == true)
      {
        exe_time = millis()-pre_time;

        if (info.count > 0)
        {
          cliPrintf("cnt : %d %3dms, ", info.count, exe_time);
          for (int i=0; i<info.count; i++)
          {
            cliPrintf(" - ");
            cliPrintf("id=%d num=%2d x=%3d y=%3d %s", 
              info.point[i].id,      
              info.point[i].num,      
              info.point[i].x, 
              info.point[i].y,
              cst816tGetGestureStr(info.point[i].id)
              );
          }
          cliPrintf("\n");
        }
      }
      else
      {
        cliPrintf("cst816tGetInfo() Fail\n");
        break;
      }
      delay(10);
    }
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("cst816t info\n");
    cliPrintf("cst816t init\n");
    cliPrintf("cst816t read addr len[0~255]\n");
    cliPrintf("cst816t write addr data \n");
    cliPrintf("cst816t get info\n");
  }
}
#endif
