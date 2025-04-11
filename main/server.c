#include "server.h"
#include "i2c.h"

#include <cJSON.h>
#include <esp_chip_info.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_vfs.h>
#include <fcntl.h>

#define HTTP_QUERY_KEY_MAX_LEN (64)
#define SCRATCH_BUFSIZE (10240)
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
static const char *SERVER_TAG = "VANMON-SERVER";

typedef struct server_context {
  char base_path[ESP_VFS_PATH_MAX + 1];
  char scratch[SCRATCH_BUFSIZE];
} server_context_t;

extern ina219_module_t module; // Get access to the module variable from i2c.c

uint8_t check_file_extension(const char *filename, const char *ext) {
  return strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0;
}

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req,
                                            const char *filepath) {
  const char *type = "text/plain";
  if (check_file_extension(filepath, ".html")) {
    type = "text/html";
  } else if (check_file_extension(filepath, ".js")) {
    type = "application/javascript";
  } else if (check_file_extension(filepath, ".css")) {
    type = "text/css";
  } else if (check_file_extension(filepath, ".png")) {
    type = "image/png";
  } else if (check_file_extension(filepath, ".ico")) {
    type = "image/x-icon";
  } else if (check_file_extension(filepath, ".svg")) {
    type = "text/xml";
  }
  return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t common_get_handler(httpd_req_t *req) {
  char filepath[FILE_PATH_MAX];

  server_context_t *rest_context = (server_context_t *)req->user_ctx;
  strlcpy(filepath, rest_context->base_path, sizeof(filepath));
  if (req->uri[strlen(req->uri) - 1] == '/') {
    strlcat(filepath, "www/index.html", sizeof(filepath));
  } else {
    strlcat(filepath, "www", sizeof(filepath));
    strlcat(filepath, req->uri, sizeof(filepath));
  }
  int fd = open(filepath, O_RDONLY, 0);
  if (fd == -1) {
    ESP_LOGE(SERVER_TAG, "Failed to open file : %s", filepath);
    /* Respond with 500 Internal Server Error */
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Failed to read existing file");
    return ESP_FAIL;
  }

  set_content_type_from_file(req, filepath);

  char *chunk = rest_context->scratch;
  ssize_t read_bytes;
  do {
    /* Read file in chunks into the scratch buffer */
    read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
    if (read_bytes == -1) {
      ESP_LOGE(SERVER_TAG, "Failed to read file : %s", filepath);
    } else if (read_bytes > 0) {
      /* Send the buffer contents as HTTP response chunk */
      if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
        close(fd);
        ESP_LOGE(SERVER_TAG, "File sending failed!");
        /* Abort sending file */
        httpd_resp_sendstr_chunk(req, NULL);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to send file");
        return ESP_FAIL;
      }
    }
  } while (read_bytes > 0);
  /* Close file after sending complete */
  close(fd);
  ESP_LOGI(SERVER_TAG, "File sending complete");
  /* Respond with an empty chunk to signal HTTP response completion */
  httpd_resp_send_chunk(req, NULL, 0);
  return ESP_OK;
}

/* Simple handler for getting system handler */
static esp_err_t system_info_get_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "application/json");
  cJSON *root = cJSON_CreateObject();
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  cJSON_AddStringToObject(root, "version", IDF_VER);
  cJSON_AddNumberToObject(root, "cores", chip_info.cores);
  const char *sys_info = cJSON_Print(root);
  httpd_resp_sendstr(req, sys_info);
  free((void *)sys_info);
  cJSON_Delete(root);
  return ESP_OK;
}

static esp_err_t data_get_handler(httpd_req_t *req) {
  char response[128];

  snprintf(response, sizeof(response),
           "{\"voltage\": %.2f, \"current\": %.2f, \"power\": %.2f}", module.voltage, module.current * 1000,
           module.power * 1000);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_status(req, "200 OK");
  httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

httpd_handle_t start_server(const char *base_path) {
  if (!base_path) {
    ESP_LOGE(SERVER_TAG, "Base path is NULL");
    return NULL;
  }

  server_context_t *rest_context = calloc(1, sizeof(server_context_t));
  if (!rest_context) {
    ESP_LOGE(SERVER_TAG, "No memory for rest context");
    return NULL;
  }

  strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  config.uri_match_fn = httpd_uri_match_wildcard;

  ESP_LOGI(SERVER_TAG, "Starting HTTP Server");
  if (httpd_start(&server, &config) != ESP_OK) {
    ESP_LOGE(SERVER_TAG, "Error starting server");
    free(rest_context);
    return NULL;
  }

  /* URI handler for fetching system info */
  httpd_uri_t system_info_get_uri = {.uri = "/api/v1/system/info",
                                     .method = HTTP_GET,
                                     .handler = system_info_get_handler,
                                     .user_ctx = rest_context};
  httpd_register_uri_handler(server, &system_info_get_uri);

  httpd_uri_t data_get_uri = {.uri = "/api/v1/data",
                              .method = HTTP_GET,
                              .handler = data_get_handler,
                              .user_ctx = rest_context};
  httpd_register_uri_handler(server, &data_get_uri);

  /* URI handler for getting web server files */
  httpd_uri_t common_get_uri = {.uri = "/*",
                                .method = HTTP_GET,
                                .handler = common_get_handler,
                                .user_ctx = rest_context};
  httpd_register_uri_handler(server, &common_get_uri);

  return server;
}

void connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                     void *event_data) {
  httpd_handle_t *server = (httpd_handle_t *)arg;
  if (*server == NULL) {
    ESP_LOGI(SERVER_TAG, "Starting webserver");
    *server = start_server("/");
  }
}
