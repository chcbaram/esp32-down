#ifndef CST816T_H_
#define CST816T_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "hw_def.h"

#ifdef _USE_HW_CST816T


#define GESTURE_NONE          0x00
#define GESTURE_SWIPE_UP      0x01
#define GESTURE_SWIPE_DOWN    0x02
#define GESTURE_SWIPE_LEFT    0x03
#define GESTURE_SWIPE_RIGHT   0x04
#define GESTURE_SINGLE_CLICK  0x05
#define GESTURE_DOUBLE_CLICK  0x0B
#define GESTURE_LONG_PRESS    0x0C

typedef struct
{
  uint8_t  id;
  uint8_t  num;
  uint8_t  event;
  uint16_t x;
  uint16_t y;
} cst816t_point_t;

typedef struct
{
  uint8_t count;
  cst816t_point_t point[5];
} cst816t_info_t;


bool cst816tInit(void);
bool cst816tGetInfo(cst816t_info_t *p_info);
uint16_t cst816tGetWidth(void);
uint16_t cst816tGetHeight(void);
const char *cst816tGetGestureStr(uint8_t id);

#endif

#ifdef __cplusplus
 }
#endif

#endif