#include "i2c.h"
#include "esp_log.h"

static const char *I2C_TAG = "VANMON-I2CBUS";

void setup_i2c_bus() { ESP_ERROR_CHECK(i2cdev_init()); }

void setup_ina219(ina219_module_t *module) {
  ESP_ERROR_CHECK(ina219_init_desc(module->dev, CONFIG_I2C_ADDR, I2C_PORT,
                                   CONFIG_I2C_MASTER_SDA,
                                   CONFIG_I2C_MASTER_SCL));
  ESP_LOGI(I2C_TAG, "Initialising INA219");
  ESP_ERROR_CHECK(ina219_init(module->dev));

  // Configure the INA219
  ESP_LOGI(I2C_TAG, "Configuring INA219");
  ESP_ERROR_CHECK(ina219_configure(module->dev, INA219_BUS_RANGE_16V, INA219_GAIN_0_125,
                                   INA219_RES_12BIT_1S, INA219_RES_12BIT_1S,
                                   INA219_MODE_CONT_SHUNT_BUS));

  // Calibrate the INA219
  ESP_LOGI(I2C_TAG, "Calibrating INA219");
  ESP_ERROR_CHECK(
      ina219_calibrate(module->dev, (float)CONFIG_SHUNT_RESISTOR_MILLI_OHM / 1000.0f));
}

void update_ina219(ina219_module_t *module) {
  ESP_ERROR_CHECK(ina219_get_bus_voltage(module->dev, &module->voltage));
//  ESP_ERROR_CHECK(ina219_get_shunt_voltage(&module->dev, &shunt_voltage));
  ESP_ERROR_CHECK(ina219_get_current(module->dev, &module->current));
  ESP_ERROR_CHECK(ina219_get_power(module->dev, &module->power));
}
