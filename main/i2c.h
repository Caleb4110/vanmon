#ifndef I2C_H
#define I2C_H

#include <ina219.h>

#define I2C_PORT 0

typedef struct ina219_module {
  ina219_t *dev;
  char *name;
  float voltage;
  float current;
  float power;
} ina219_module_t;

void setup_i2c_bus();

void setup_ina219(ina219_module_t *module);

void update_ina219(ina219_module_t *module);

#endif // I2C_H
