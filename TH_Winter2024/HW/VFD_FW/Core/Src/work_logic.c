#include "work_logic.h"
#include <string.h>
#include <stdlib.h>
#include "LoRa1.h"
#include "glonass.h"
//--------------------------------------------------------------------------------------------------



uint32_t       led_sequencer = 0x00FF00FFul;
bme280_t       BME_sensor;
GPS_Data_Struct       GPS_Data;

LoRa myLoRa;
uint8_t frame[27];

extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;

uint8_t pack_sent = 0;

uint8_t float_arr[4];
uint8_t *ptr = float_arr;


uint8_t              NMEAC;          
uint8_t              NMEAF; 
extern uint8_t NMEAFlag;
extern uint8_t NMEACount;
uint32_t Lat_GPS;
uint32_t Long_GPS;


void PreLoop(void){
		HAL_Delay(1000);
		BME_get_ID(&hi2c1, (MAIN_BME_ADDRESS<<1), &BME_sensor);
		BME_get_calib(&hi2c1, (MAIN_BME_ADDRESS<<1), &BME_sensor);

		bme280_settings_t bme_settings;
		bme_settings.mode = NORMAL_MODE;
		bme_settings.t_stb = TIME_STB_0_5;
		bme_settings.ors_t = OSAMPLING_2;
		bme_settings.ors_p = OSAMPLING_16;
		bme_settings.ors_h = OSAMPLING_1;
		bme_settings.filter = FILTER_16;
		BME_set_settings(&hi2c1, (MAIN_BME_ADDRESS<<1), bme_settings);
		//bme_struct_init(&hi2c1, &BME_sensor);

		myLoRa.hSPIx                 = &hspi1;
		myLoRa.CS_port               = GPIOA;
		myLoRa.CS_pin                = GPIO_PIN_3;
		myLoRa.reset_port            = GPIOA;
		myLoRa.reset_pin             = GPIO_PIN_10;
		myLoRa.DIO0_port						 = GPIOA;
		myLoRa.DIO0_pin							 = GPIO_PIN_8;

		myLoRa.frequency             = 436;							  // default = 433 MHz
		myLoRa.spredingFactor        = SF_10;							// default = SF_7
		myLoRa.bandWidth			       = BW_125KHz;				  // default = BW_125KHz
		myLoRa.crcRate				       = CR_4_5;						// default = CR_4_5
		myLoRa.power					       = POWER_20db;				// default = 20db
		myLoRa.overCurrentProtection = 100; 							// default = 100 mA
		myLoRa.preamble				       = 8;		

		LoRa_reset(&myLoRa);
		LoRa_init(&myLoRa);
		//LoRa_startReceiving(&myLoRa);

		GLNS_InitStr();
		//IRQ enable
		huart1.Instance->SR &= ~ (USART_SR_TC | USART_SR_RXNE);
		huart1.Instance->DR;
		huart1.Instance->CR1 |= USART_CR1_RXNEIE;

		frame[0] = 0xFE;
		frame[26] = 0xEF;

		frame[1] = 1;
		frame[14] = 2;
		
}

//--------------------------------------------------------------------------------------------------

void Loop(void){
	static uint32_t start_time = 0;

	
	 if(HAL_GetTick() - start_time > 2000) {
		 start_time = HAL_GetTick();
			
		 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);

		 I2C_dev_machine();
		 gps_logic();
		 push_data();
		 LoRa_transmit(&myLoRa, frame, 27, 1000);
		 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	 
	 }
}

void gps_logic(){
	static uint32_t start_time = 0;

	if (NMEAFlag == MSG_RMC){
         GLNS_NMEADataConvertRMC();

         if(GPS_Data.Valid){
            start_time = HAL_GetTick();
         }
         else {
            //do reboot GLONASS protothread when 'valid' flag is lost
            if((HAL_GetTick() - start_time) > GLONASS_VALID_TIMEOUT){
               start_time = HAL_GetTick();
               //PT_EXIT(pt);
            }
         }

         NMEAFlag = 0;
         NMEACount = 0;
         //GPS_Data.pack_bufNum = 0;
      }
      if (NMEAFlag == MSG_GGA){
         
         GLNS_NMEADataConvertGGA();
         
         NMEACount = 0;
         NMEAFlag = 0;
      }
	
}

//--------------------------------------------------------------------------------------------------
void push_data(){
	
	 ptr = (uint8_t*)(&BME_sensor.data.press);
	 memcpy(frame+2, ptr, 4);

   ptr = (uint8_t*)(&BME_sensor.data.temp);
	 memcpy(frame+6, ptr, 4);

   ptr = (uint8_t*)(&BME_sensor.data.hum);
	 memcpy(frame+10, ptr, 4);

	
	 //GPS_Data.Long = 0x02F8B86D;
	 ptr = (uint8_t*)(&GPS_Data.Long);
	 memcpy(frame+15, ptr, 4);
	
	 //GPS_Data.Lat = 0x01F61B9A;
	 ptr = (uint8_t*)(&GPS_Data.Lat);
	 memcpy(frame+19, ptr, 4);
	
	 //frame[22] = 0xFF;
	 //frame[18] = 0xFF;
	
	 frame[23] = GPS_Data.Hour;
	 frame[24] = GPS_Data.Min;
	 frame[25] = GPS_Data.Sec;
	
   //char buffer[] = "digital_gvn";
   //memcpy(frame+15, buffer, strlen(buffer));
	
}



float fixed_to_float(uint32_t data, uint8_t fractional_count) {
	uint32_t integer_part = data >> fractional_count;
	uint32_t fractional_part = data & ((1 << fractional_count) - 1);
	
	return (float)integer_part + (float)fractional_part/(1 << fractional_count);
}

