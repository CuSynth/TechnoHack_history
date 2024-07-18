#include "i2c_dev.h"

extern I2C_HandleTypeDef      hi2c1;
extern bme280_t               BME_sensor;

void I2C_dev_machine(void){

	BME_get_status(&hi2c1, (MAIN_BME_ADDRESS<<1), &BME_sensor);
	// Check status
	BME_get_raw_data(&hi2c1, (MAIN_BME_ADDRESS<<1), &BME_sensor);
	BME_process_data(&BME_sensor);
}
