#ifndef SERVER_H
#define SERVER_H

#include <esp_err.h>
#include <esp_http_server.h>

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);

void connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                     void *event_data);

#endif // SERVER_H
