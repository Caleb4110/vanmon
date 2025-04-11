#ifndef WIFI_H
#define WIFI_H

#define ESP_WIFI_SSID "VanMon"
#define ESP_WIFI_PASS "VanMon@123"
#define ESP_WIFI_CHANNEL CONFIG_ESP_WIFI_CHANNEL
#define MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN

void wifi_init_softap(void);

#endif // WIFI_H
