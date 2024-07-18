#ifndef __I2C_DEV__
#define __I2C_DEV__

#include "BME280.h"
#include "main.h"

#define MAIN_BME_ADDRESS               BME280_ADDRESS 
#define I2C_MACHINE_START_DELAY        1000

void I2C_dev_machine(void);

#endif
