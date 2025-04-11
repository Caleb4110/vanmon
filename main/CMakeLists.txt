idf_component_register(SRCS "main.c" "wifi.c" "server.c" "i2c.c"
                    PRIV_REQUIRES esp_wifi nvs_flash esp_wifi esp_eth esp-tls nvs_flash esp_netif esp_http_server ina219 json spiffs
                    INCLUDE_DIRS ".")

set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../frontend")
    if(EXISTS ${WEB_SRC_DIR}/dist)
        spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
        message(STATUS "SPIFFS partition image created at ${WEB_SRC_DIR}/dist")
    else()
        message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
    endif()


