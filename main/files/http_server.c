#include <stdio.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "../header/boom_barrier.h"
#include "../header/http_server.h"
#include "../header/spiffs_create.h"
#include "esp_system.h"
#include "cJSON.h"

static const char *HTTP_TAG = "file_server";

void slice(const char *str, char *result, size_t start, size_t end){
    ESP_LOGI(HTTP_TAG, "The string passed in is: %s and it's length is: %d", str, strlen(str));
	strncpy(result, str + start, strlen(str)-start+end);
}


/* Handler to redirect incoming GET request for /index.html to /
 * This can be overridden by uploading file with same name */
static esp_err_t index_html_handler(httpd_req_t *req)
{
    extern const unsigned char index_html_start[]   asm("_binary_index_html_start");
    extern const unsigned char index_html_end[]     asm("_binary_index_html_end");
    const size_t index_html_size = (index_html_end - index_html_start);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char*) index_html_start, index_html_size);  // Response body can be empty
    return ESP_OK;
}


/* Handler to respond with an icon file embedded in flash.
 * Browsers expect to GET website icon at URI /favicon.ico.
 * This can be overridden by uploading file with same name */
static esp_err_t images_png_get_handler(httpd_req_t *req)
{
    extern const unsigned char favicon_ico_start[] asm("_binary_images_png_start");
    extern const unsigned char favicon_ico_end[]   asm("_binary_images_png_end");
    const size_t favicon_ico_size = (favicon_ico_end - favicon_ico_start);
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_size);
    return ESP_OK;
}


static esp_err_t main_js_handler(httpd_req_t *req){
    extern const unsigned char main_js_start[]      asm("_binary_main_js_start");
    extern const unsigned char main_js_end[]        asm("_binary_main_js_end");
    const size_t main_js_size = (main_js_end - main_js_start);
    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)main_js_start, main_js_size);
    return ESP_OK;
}

static esp_err_t main_css_handler(httpd_req_t *req){
    extern const unsigned char main_css_start[]      asm("_binary_main_css_start");
    extern const unsigned char main_css_end[]        asm("_binary_main_css_end");
    const size_t main_css_size = (main_css_end - main_css_start);
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)main_css_start, main_css_size);
    return ESP_OK;
}

static esp_err_t ota_file_handler(httpd_req_t *req){
    extern const unsigned char main_ota_html_start[]    asm("_binary_ota_html_start");
    extern const unsigned char main_ota_html_end[]      asm("_binary_ota_html_end");
    const size_t main_ota_html_size = (main_ota_html_end - main_ota_html_start);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)main_ota_html_start, main_ota_html_size);
    return ESP_OK;
}

static esp_err_t index_js_handler(httpd_req_t *req){
    extern const unsigned char index_html_js_start[]   asm("_binary_index_html_js_start");
    extern const unsigned char index_html_js_end[]     asm("_binary_index_html_js_end");
    const size_t index_js_size = index_html_js_end - index_html_js_start;
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char*)index_html_js_start, index_js_size);
    return ESP_OK;
}

static esp_err_t close_barrier_post_handler(httpd_req_t *req){

    char return_response[100];

    ESP_LOGI(HTTP_TAG, "Close barrier handler called");

    sprintf(return_response, "{\"status\": \"success\", \"message\": \"Boom barrier closing.\"}");

    httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, return_response, strlen(return_response));

    ESP_LOGW(HTTP_TAG, "The value read is: %d", read_time());
    down_function(read_time());
    stop_function();
    return ESP_OK;
}

static esp_err_t open_barrier_post_handler(httpd_req_t *req) {
    char return_response[100];

    ESP_LOGI(HTTP_TAG, "Open barrier handler called");

    sprintf(return_response, "{\"status\": \"success\", \"message\": \"Boom barrier opening.\"}");

    httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, return_response, strlen(return_response));

    ESP_LOGW(HTTP_TAG, "The value read is: %d", read_time());
    up_function(read_time());
    stop_function();

    return ESP_OK;
}

static esp_err_t ota_update_handler(httpd_req_t *req){
    esp_ota_handle_t ota_handle;

	char ota_buff[1024];
	int content_length = req->content_len;
	int content_received = 0;
	int recv_len;
	bool is_req_body_started = false;
	bool flash_successful = false;

	const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

	do
	{
		// Read the data for the request
		if ((recv_len = httpd_req_recv(req, ota_buff, MIN(content_length, sizeof(ota_buff)))) < 0)
		{
			// Check if timeout occurred
			if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
			{
				ESP_LOGI(HTTP_TAG, "http_server_OTA_update_handler: Socket Timeout");
				continue; ///> Retry receiving if timeout occurred
			}
			ESP_LOGI(HTTP_TAG, "http_server_OTA_update_handler: OTA other Error %d", recv_len);
			return ESP_FAIL;
		}
		printf("http_server_OTA_update_handler: OTA RX: %d of %d\r", content_received, content_length);

		// Is this the first data we are receiving
		// If so, it will have the information in the header that we need.
		if (!is_req_body_started)
		{
			is_req_body_started = true;

			// Get the location of the .bin file content (remove the web form data)
			char *body_start_p = strstr(ota_buff, "\r\n\r\n") + 4;
			int body_part_len = recv_len - (body_start_p - ota_buff);

			printf("http_server_OTA_update_handler: OTA file size: %d\r\n", content_length);

			esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
			if (err != ESP_OK)
			{
				printf("http_server_OTA_update_handler: Error with OTA begin, cancelling OTA\r\n");
				return ESP_FAIL;
			}
			else
			{
				printf("http_server_OTA_update_handler: Writing to partition subtype %d at offset 0x%lu\r\n", update_partition->subtype, update_partition->address);
			}

			// Write this first part of the data
			esp_ota_write(ota_handle, body_start_p, body_part_len);
			content_received += body_part_len;
		}
		else
		{
			// Write OTA data
			esp_ota_write(ota_handle, ota_buff, recv_len);
			content_received += recv_len;
		}

	} while (recv_len > 0 && content_received < content_length);

	if (esp_ota_end(ota_handle) == ESP_OK)
	{
		// Lets update the partition
		if (esp_ota_set_boot_partition(update_partition) == ESP_OK)
		{
			const esp_partition_t *boot_partition = esp_ota_get_boot_partition();
			ESP_LOGI(HTTP_TAG, "http_server_OTA_update_handler: Next boot partition subtype %d at offset 0x%lu", boot_partition->subtype, boot_partition->address);
			flash_successful = true;
		}
		else
		{
			ESP_LOGI(HTTP_TAG, "http_server_OTA_update_handler: FLASHED ERROR!!!");
		}
	}
	else
	{
		ESP_LOGI(HTTP_TAG, "http_server_OTA_update_handler: esp_ota_end ERROR!!!");
	}

	// We won't update the global variables throughout the file, so send the message about the status
	if (flash_successful) { 
        ESP_LOGI(HTTP_TAG, "Flash Successful");
        esp_restart();
     } else {
        ESP_LOGI(HTTP_TAG, "Flash failed");
     }

	return ESP_OK;
}


static esp_err_t change_barrier_time_handler(httpd_req_t *req){
    char content[100];

    // ESP_LOGI(HTTP_TAG, "The HTTP request is: %s", req->uri);
    // ESP_LOGI(HTTP_TAG, "The HTTP method is: %d", req->method);
    // ESP_LOGI(HTTP_TAG, "The HTTP content length is: %d", req->content_len);


    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);

    ESP_LOGI(HTTP_TAG, "The HTTP content is: %s", content);

    cJSON *barrier_time_json = cJSON_Parse(content);

    const cJSON *time = cJSON_GetObjectItemCaseSensitive(barrier_time_json, "barrier_timing");

    ESP_LOGI(HTTP_TAG, "Converting the time in integer we get: %d", atoi(time->valuestring));

    int new_time = atoi(time->valuestring);

    barrier_time_change(new_time);

    ESP_LOGW(HTTP_TAG, "The new time read is: %d", read_time());

    return ESP_OK;
}


void http_server_start(){

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers = 12;

    ESP_LOGI(HTTP_TAG, "starting HTTP Server on port: '%d'", config.server_port);
    if(httpd_start(&server, &config) != ESP_OK){
        ESP_LOGE(HTTP_TAG, "Failed to start the file server!");
        return;
    }

    httpd_uri_t index_html = {
        .uri        = "/",
        .method     = HTTP_GET,
        .handler    = index_html_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &index_html);

    httpd_uri_t main_css = {
        .uri        = "/main.css",
        .method     = HTTP_GET,
        .handler    = main_css_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &main_css);

    httpd_uri_t main_js = {
        .uri        = "/main.js",
        .method     = HTTP_GET,
        .handler    = main_js_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &main_js);

    httpd_uri_t images_png = {
        .uri        = "/images.png",
        .method     = HTTP_GET,
        .handler    = images_png_get_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &images_png);

    httpd_uri_t favicon = {
        .uri        = "/favicon.ico",
        .method     = HTTP_GET,
        .handler    = images_png_get_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &favicon);

    httpd_uri_t close_barrier = {
        .uri        = "/close",
        .method     = HTTP_POST,
        .handler    = close_barrier_post_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &close_barrier);

    httpd_uri_t open_barrier = {
        .uri        = "/open",
        .method     = HTTP_POST,
        .handler    = open_barrier_post_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &open_barrier);

    httpd_uri_t barrier_timing = {
        .uri        = "/barrier_timing",
        .method     = HTTP_POST,
        .handler    = change_barrier_time_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &barrier_timing);

    httpd_uri_t ota_page = {
        .uri        = "/ota_update",
        .method     = HTTP_GET,
        .handler    = ota_file_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &ota_page);

    httpd_uri_t index_js = {
        .uri        = "/index_html.js",
        .method     = HTTP_GET,
        .handler    = index_js_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &index_js);

    httpd_uri_t ota_update = {
        .uri        = "/OTAupdate",
        .method     = HTTP_POST,
        .handler    = ota_update_handler,
        .user_ctx   = NULL
    };

    httpd_register_uri_handler(server, &ota_update);
}


