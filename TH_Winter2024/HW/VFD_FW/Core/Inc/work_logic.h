#ifndef _CAN_PROTOCOL_
#define _CAN_PROTOCOL_

#include "stm32f1xx.h"

#include "bme280.h"
#include "i2c_dev.h"


#define LED_OFF()                       GPIOC->BSRR = 1UL << 13;
#define LED_ON()                      GPIOC->BSRR = 1UL << (13 + 16);

#pragma pack(push, 1)

typedef struct {
   uint8_t     start;      //0xFF

   uint32_t    time;       // Time, ms
   uint16_t    T1HIG;      // 0xAAAA
   float       T1;         // Temp
   uint16_t    T2HIG;      // 0xBBBB
   float       T2;         // Temp
   uint16_t    T3HIG;      // 0xCCCC
   float       T3;         // Temp
   float       DHT;        // Hum
   uint8_t     solid_hum;  // 0xDD
   uint8_t     vel;        // 0xEE
   
   uint8_t     end;        // 0xF3
} package_t;

#pragma pack(pop)


//--------------------------------------------------------------------------------------------------

void           PreLoop(void);
void           Loop(void);



//--------------------------------------------------------------------------------------------------
void           push_data(void);
void					 gps_logic(void);
float fixed_to_float(uint32_t data, uint8_t fractional_count);

#endif
