#ifndef _BME_280_ // include guard
#define _BME_280_

#include "stm32f1xx.h"
#include "pt.h"

#define HPA_TO_MMHG           (0.750062)

#define BME280_ADDRESS        0x76
#define BME280_SDO_BIT        0x01

//--------------------------------------------------------------------------------------------------
// BME280 registers
#define BME_CALIB_0_REG       0x88  // calib0..cali25 are in 0x88..0xA1
#define BME_ID_REG            0xD0
#define BME_RESET_REG         0xE0  
#define BME_CALIB_26_REG      0xE1  // calib26..cali41 are in 0xE1..0xF0
#define BME_CTRL_HUM_REG      0xF2
#define BME_STATUS_REG        0xF3
#define BME_CTRL_MES_REG      0xF4
#define BME_CONFIG_REG        0xF5
#define BME_PRESS_MSB_REG     0xF7
#define BME_PRESS_LSB_REG     0xF8
#define BME_PRESS_XLSB_REG    0xF9
#define BME_TEMP_MSB_REG      0xFA
#define BME_TEMP_LSB_REG      0xFB
#define BME_TEMP_XLSB_REG     0xFC
#define BME_HUM_MSB_REG       0xFD
#define BME_HUM_LSB_REG       0xFE

#define RESET_VALUE           0xB6

//--------------------------------------------------------------------------------------------------
// Oversampling settings

#define SKIPPED               0x00
#define OSAMPLING_1           0x01
#define OSAMPLING_2           0x02
#define OSAMPLING_4           0x03
#define OSAMPLING_8           0x04
#define OSAMPLING_16          0x05

//--------------------------------------------------------------------------------------------------
// Modes

#define BM_SLEEP_MODE            0x00
#define FORCED_MODE           0x01
#define NORMAL_MODE           0x03

//--------------------------------------------------------------------------------------------------
// Time standby in ms

#define TIME_STB_0_5          0x00
#define TIME_STB_62_5         0x01
#define TIME_STB_125          0x02
#define TIME_STB_250          0x03
#define TIME_STB_500          0x04
#define TIME_STB_1000         0x05
#define TIME_STB_10           0x06
#define TIME_STB_20           0x07

//--------------------------------------------------------------------------------------------------
// Filter coefficient

#define FILTER_OFF            0x00
#define FILTER_2              0x01
#define FILTER_4              0x02
#define FILTER_8              0x03
#define FILTER_16             0x04

//--------------------------------------------------------------------------------------------------
// Status mask

#define MEASURING_MASK        (1U << 3)
#define IM_UPDATE_MASK        (1U << 0)

//--------------------------------------------------------------------------------------------------

#pragma pack(push, 1)

typedef struct
{//                                                offsets
   uint8_t        ID;                              //+0
   
   float          hum;  	 //%RH   (Q32.10)       //1
   float          temp;     //*100  deg C          //5
   float          press;    //Pa		(Q24.8)        //9
} bme_data_t;

typedef struct
{//                                                offsets
   uint8_t        pres_msb;                         //+0   
   uint8_t        pres_lsb;                         //1   
   uint8_t        pres_xlsb;                        //2   
   
   uint8_t        temp_msb;                         //3   
   uint8_t        temp_lsb;                         //4   
   uint8_t        temp_xlsb;                        //5   
   
   uint8_t        hum_msb;                          //6   
   uint8_t        hum_lsb;                          //7   
   
} bme_raw_data_t;

typedef struct
{//                                                offsets
   uint16_t       dig_T1;                           //+0
   int16_t        dig_T2;                           //2
   int16_t        dig_T3;                           //4
   
   uint16_t       dig_P1;                           //6
   int16_t        dig_P2;                           //8
   int16_t        dig_P3;                           //10
   int16_t        dig_P4;                           //12
   int16_t        dig_P5;                           //14
   int16_t        dig_P6;                           //16
   int16_t        dig_P7;                           //18
   int16_t        dig_P8;                           //20
   int16_t        dig_P9;                           //22

   uint8_t        dig_H1;                           //24
   int16_t        dig_H2;                           //25
   uint8_t        dig_H3;                           //27
   int16_t        dig_H4;                           //28
   int16_t        dig_H5;                           //30
   int8_t         dig_H6;                           //32
} bme_cal_t;

typedef struct
{//                                                offsets
   bme_raw_data_t raw_data;                        //+0   
   bme_data_t     data;                            //8
   bme_cal_t      calib;                           //19
   uint8_t        status;                          //51
} bme280_t;


typedef struct
{//                                                offsets
   uint8_t        mode;                            //+0

   uint8_t        t_stb;                           //1
   uint8_t        ors_t;                           //2
   uint8_t        ors_p;                           //3     
   uint8_t        ors_h;                           //4

   uint8_t        filter;                          //5

} bme280_settings_t;

#pragma pack(pop)

//--------------------------------------------------------------------------------------------------



uint8_t BME_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, uint8_t addr, uint8_t val);
uint8_t BME_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, uint8_t addr, uint8_t* val, unsigned len);

uint8_t BME_get_ID(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_t* bme);
uint8_t BME_get_calib(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_t* bme);
uint8_t BME_get_status(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_t* bme);
uint8_t BME_get_raw_data(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_t* bme);
void BME_process_data(bme280_t* bme);
float BME_process_temp(bme280_t* bme, int32_t* t_fine);
float BME_process_pres(bme280_t* bme, int32_t* t_fine);
float BME_process_hum(bme280_t* bme, int32_t* t_fine);

uint8_t BME_set_mode(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, uint8_t mode);
uint8_t BME_set_settings(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_settings_t set);

uint8_t BME_reset(I2C_HandleTypeDef *hi2c, uint8_t bme_addr);



#endif
