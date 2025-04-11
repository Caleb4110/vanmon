#include "i2c.h"
#include "server.h"
#include "wifi.h"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_spiffs.h>
#include <nvs_flash.h>

static const char *TAG = "VANMON";

ina219_module_t module;

esp_err_t init_fs(void) {
  esp_vfs_spiffs_conf_t conf = {.base_path = CONFIG_WEB_MOUNT_POINT,
                                .partition_label = "www",
                                .max_files = 5,
                                .format_if_mount_failed = false};
  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG, "Failed to find SPIFFS partition");
    } else {
      ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
    }
    return ESP_FAIL;
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info("www", &total, &used);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)",
             esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
  }
  return ESP_OK;
}

void app_main(void) {

  ina219_t dev;
  memset(&dev, 0, sizeof(ina219_t));
  memset(&module, 0, sizeof(ina219_module_t));
  module.name = "Fridge";
  module.dev = &dev;
  module.voltage = 0;
  module.current = 0;
  module.power = 0;

  setup_i2c_bus();

  setup_ina219(&module);

  update_ina219(&module);

  printf("VBUS: %.04f V, IBUS: %.04f mA, PBUS: %.04f mW\n", module.voltage,
         module.current * 1000, module.power * 1000);

  // Server initialisation
  static httpd_handle_t server = NULL;
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());

  ESP_LOGI(TAG, "VANMON V1.0: AP mode");
  wifi_init_softap();

  ESP_ERROR_CHECK(init_fs());
  ESP_ERROR_CHECK(esp_event_handler_register(
      IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    update_ina219(&module);
    //printf("VBUS: %.04f V, IBUS: %.04f mA, PBUS: %.04f mW\n", module.voltage,
     //      module.current * 1000, module.power * 1000);
  }
}
