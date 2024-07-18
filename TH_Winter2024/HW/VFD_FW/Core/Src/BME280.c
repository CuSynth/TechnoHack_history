#include "BME280.h"
#include "work_logic.h"
#include <string.h>


void BME_process_data(bme280_t* bme) {
   //---------------------------------
   // Temp
   
   int32_t t_fine;
   float temperature = BME_process_temp(bme, &t_fine);
   bme->data.temp = temperature;
   
   //---------------------------------
   // Pres
   float pres = BME_process_pres(bme, &t_fine);
   bme->data.press = (pres/100)*HPA_TO_MMHG;

   //---------------------------------
   // Pres
   float hum = BME_process_hum(bme, &t_fine);
   bme->data.hum = hum;   
}

//--------------------------------------------------------------------------------------------------

float BME_process_temp(bme280_t* bme, int32_t* t_fine) {
   uint32_t adc_T = (bme->raw_data.temp_msb<<12) 
                  | (bme->raw_data.temp_lsb<<4) 
                  | ((bme->raw_data.temp_xlsb >> 4) & 0x0F);

   int32_t var1, var2, T; 
   var1 =  ((((adc_T>>3) - ((int32_t)bme->calib.dig_T1 << 1))) * ((int32_t)bme->calib.dig_T2))>>11;
   var2 = (((((adc_T>>4) - ((int32_t)bme->calib.dig_T1)) * ((adc_T>>4) - ((int32_t)bme->calib.dig_T1))) >> 12) * ((int32_t)bme->calib.dig_T3)) >> 14; 
   
   *t_fine = var1 + var2;
   T = (*t_fine * 5 + 128) >> 8; 
   return (float)T*0.01;
}

float BME_process_pres(bme280_t* bme, int32_t* t_fine) {
    int32_t adc_P = (bme->raw_data.pres_msb<<12) 
                  | (bme->raw_data.pres_lsb<<4) 
                  | ((bme->raw_data.pres_xlsb >> 4) & 0x0F);
   
	int64_t var1, var2, p;
   var1 = (*t_fine) - 128000; 
	var2 = var1 * var1 * bme->calib.dig_P6; 
	var2 = var2 + ((var1 * bme->calib.dig_P5)<<17); 
   uint64_t dig_P4 = bme->calib.dig_P4;
	var2 = var2 + ((dig_P4)<<35); 
	var1 = ((var1 * var1 * bme->calib.dig_P3)>>8) + ((var1 * bme->calib.dig_P2)<<12); 
	var1 = ((((int64_t)1)<<47)+var1)*((int64_t)bme->calib.dig_P1)>>33; 
	if (var1 == 0) { 
		return 0; // avoid exception caused by division by zero 
	} 
	p = 1048576-adc_P; 
	p = (((p<<31)-var2)*3125)/var1; 
	var1 = (((int64_t)bme->calib.dig_P9) * (p>>13) * (p>>13)) >> 25; 
	var2 = (((int64_t)bme->calib.dig_P8) * p) >> 19; p = ((p + var1 + var2) >> 8) + (((int64_t)bme->calib.dig_P7)<<4); 
	return fixed_to_float((int32_t)p, 8);	
}

float BME_process_hum(bme280_t* bme, int32_t* t_fine) {
   
   int32_t adc_H = (bme->raw_data.hum_msb<<8) 
                  | ((bme->raw_data.hum_lsb) & 0xFF);
   
   int32_t v_x1_u32r;
   v_x1_u32r = (*t_fine - ((int32_t)76800));
   v_x1_u32r = (((((adc_H << 14) - (((int32_t)bme->calib.dig_H4) << 20) - (((int32_t)bme->calib.dig_H5) * 
               v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * 
               ((int32_t)bme->calib.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)bme->calib.dig_H3)) >> 11) + 
               ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)bme->calib.dig_H2) + 
               8192) >> 14)); 
   v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)bme->calib.dig_H1)) >> 4)); 
   v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r); 
   v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r); 
   return fixed_to_float((int32_t)(v_x1_u32r>>12), 10);
}

//--------------------------------------------------------------------------------------------------

// Todo: move to i2c dev
uint8_t BME_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, uint8_t addr, uint8_t val) {
   // Todo: refactor
   uint8_t buf[2];   
   buf[0] = addr;
   buf[1] = val;

   if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, bme_addr, buf, 2U, 100U))
      return 1;
   return 0;

}


uint8_t BME_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, uint8_t addr, uint8_t * reg, unsigned len) {
   
   if (HAL_OK != HAL_I2C_Master_Transmit(hi2c, bme_addr, &addr, 1U, 100U))
      return 1;
   if (HAL_OK != HAL_I2C_Master_Receive(hi2c, bme_addr, reg, len, 100U))
      return 1;
   return 0;
}

//--------------------------------------------------------------------------------------------------

uint8_t BME_get_ID(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_t* bme) {
   return BME_ReadRegister(hi2c, bme_addr, BME_ID_REG, &bme->data.ID, 1U);
}

uint8_t BME_get_calib(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_t* bme) {

   uint8_t buff[24];
   uint8_t ret = 0;
   uint8_t dat = 0;
   
   for(int i=0; i<24; ++i) {
      ret = BME_ReadRegister(hi2c, bme_addr, (BME_CALIB_0_REG+i), (buff+i), 1U);
      if(ret)
         return ret;
   }
   
   memcpy(&(bme->calib.dig_T1), buff, 24);

   if(BME_ReadRegister(hi2c, bme_addr, (BME_CALIB_0_REG+25), &(bme->calib.dig_H1), 1U)) {
      return 1;
   }

   for(int i=0; i<3; ++i) {
      ret = BME_ReadRegister(hi2c, bme_addr, (BME_CALIB_26_REG+i), (buff+i), 1U);
      if(ret)
         return ret;
   }
   
   memcpy(&(bme->calib.dig_H2), buff, 3);
   
   if(BME_ReadRegister(hi2c, bme_addr, (BME_CALIB_26_REG+3), &dat, 1U)) {
      return 1;
   }
   
   bme->calib.dig_H4 = (dat<<4);
   
   if(BME_ReadRegister(hi2c, bme_addr, (BME_CALIB_26_REG+4), &dat, 1U)) {
      return 1;
   }
   
   bme->calib.dig_H4 |= (dat&0x0F);
   bme->calib.dig_H5 = 0;
   bme->calib.dig_H5 |= ((dat>>4)&0x0F);

   if(BME_ReadRegister(hi2c, bme_addr, (BME_CALIB_26_REG+5), &dat, 1U)) {
      return 1;
   }

   bme->calib.dig_H5 |= (dat<<4);

   if(BME_ReadRegister(hi2c, bme_addr, (BME_CALIB_26_REG+6), &dat, 1U)) {
      return 1;
   }
   bme->calib.dig_H6 = dat;
   
   return 0;
}

uint8_t BME_get_status(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_t* bme) {
   return BME_ReadRegister(hi2c, bme_addr, BME_STATUS_REG, &(bme->status), 1U);
}

uint8_t BME_get_raw_data(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_t* bme)  {
   return BME_ReadRegister(hi2c, bme_addr, BME_PRESS_MSB_REG, &(bme->raw_data.pres_msb), 8U);
}


uint8_t BME_set_mode(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, uint8_t mode) {

   return BME_WriteRegister(hi2c, bme_addr, BME_CTRL_MES_REG, mode&0x03);
}

uint8_t BME_set_settings(I2C_HandleTypeDef *hi2c, uint8_t bme_addr, bme280_settings_t set) {
   
   int ret = 0;
   uint8_t cfg = ((set.ors_t&0x07)<<5) | ((set.ors_p&0x07)<<2) | (set.mode&0x03);
   
   ret = BME_WriteRegister(hi2c, bme_addr, BME_CTRL_MES_REG, cfg);
   if(ret) {
      return ret;
   }

   cfg = ((set.t_stb&0x07)<<5) | ((set.filter&0x07)<<2);
   ret = BME_WriteRegister(hi2c, bme_addr, BME_CONFIG_REG, cfg);
   if(ret) {
      return ret;
   }
   
   ret = BME_WriteRegister(hi2c, bme_addr, BME_CTRL_HUM_REG, set.mode&0x07);
   if(ret) {
      return ret;
   }
 
   return 0;
}

uint8_t BME_reset(I2C_HandleTypeDef *hi2c, uint8_t bme_addr) {
   return BME_WriteRegister(hi2c, bme_addr, BME_RESET_REG, RESET_VALUE);
}



