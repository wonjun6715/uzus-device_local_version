#include <string.h>
#include <stdio.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mbedtls/base64.h"

#include "../inc/wifi.h"
#include "../inc/file.h"
#include "../inc/mqtt.h"
#include "../inc/common.h"

#define STORAGE_NAMESPACE "storage"

static const char* TAG = "HTTP";

/********************************************************************************
* HTTPServerRootHandler
* Parameter 
*   req : HTTP 요청에 대한 포인터 변수
* 
* Description : / 요청에 대한 핸들러 함수
********************************************************************************/
static esp_err_t HTTPServerRootHandler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "[HTTPServerRootHandler] Start");

    if(req == NULL) {
        ESP_LOGI(TAG, "[HTTPServerRootHandler] Server is Null");

        return ESP_FAIL;
    }
	
    ESP_LOGI(TAG, "[HTTPServerRootHandler] URI : [%s]", req->uri);

    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html>");
    httpd_resp_sendstr_chunk(req, "<head></head>");
    httpd_resp_sendstr_chunk(req, "<meta charset=""utf-8"">");
    httpd_resp_sendstr_chunk(req, "</head>");
    httpd_resp_sendstr_chunk(req, "<body>");
    
    httpd_resp_sendstr_chunk(req, "<h1>UzUs Wifi Setting</h1>");
    httpd_resp_sendstr_chunk(req, "<form method=\"post\" action=\"/post\">");
    httpd_resp_sendstr_chunk(req, "사용자 이름 <input type=\"text\" name=\"user\" value=\"");
	httpd_resp_sendstr_chunk(req, "\">");
	httpd_resp_sendstr_chunk(req, "<br>");
    httpd_resp_sendstr_chunk(req, "<br>");
	httpd_resp_sendstr_chunk(req, "WiFi 정보 <input type=\"text\" name=\"wifi\" value=\"");
	httpd_resp_sendstr_chunk(req, "\">");
	httpd_resp_sendstr_chunk(req, "<br>");
    httpd_resp_sendstr_chunk(req, "<br>");
	httpd_resp_sendstr_chunk(req, "비밀번호: <input type=\"text\" name=\"password\" value=\"");
	httpd_resp_sendstr_chunk(req, "\">");
	httpd_resp_sendstr_chunk(req, "<br>");
    httpd_resp_sendstr_chunk(req, "<br>");
	httpd_resp_sendstr_chunk(req, "<input type=\"submit\" name=\"submit\" value=\"설정\">");

	httpd_resp_sendstr_chunk(req, "</form><br>");
	httpd_resp_sendstr_chunk(req, "<hr>");

#if DEBUG
    httpd_resp_sendstr_chunk(req, "<form method=\"post\" action=\"/test\">");
	httpd_resp_sendstr_chunk(req, "<br>");
    httpd_resp_sendstr_chunk(req, "<br>");
	httpd_resp_sendstr_chunk(req, "<button onclick=\"window.location.href='/test'\">Data Send</button>");
    httpd_resp_sendstr_chunk(req, "\">");
    httpd_resp_sendstr_chunk(req, "</form><br>");
#endif

    httpd_resp_sendstr_chunk(req, "</body></html>");

    httpd_resp_sendstr_chunk(req, NULL);

    ESP_LOGI(TAG, "[HTTPServerRootHandler] End");

    return ESP_OK;
}

/********************************************************************************
* HTTPServerErrorHandler
* Parameter 
*   req : HTTP 요청에 대한 포인터 변수
* 
* Description : 404 Error 발생 시 호출되는 Event Handler
********************************************************************************/

esp_err_t HTTPServerErrorHandler(httpd_req_t* req)
{
    ESP_LOGI(TAG, "[HTTPServerErrorHandler] Start");
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "404 Error Message");

    ESP_LOGI(TAG, "[HTTPServerErrorHandler] End");

    return ESP_FAIL;
}


/********************************************************************************
* HTTPServerPostHandler
* Parameter 
*   req : HTTP 요청에 대한 포인터 변수
* 
* Description : /post 요청에 대한 핸들러 함수
********************************************************************************/

static esp_err_t HTTPServerPostHandler(httpd_req_t *req)
{   
	ESP_LOGI(TAG, "[HTTPServerPostHandler] Start");

    ESP_LOGI(TAG, "[HTTPServerPostHandler] URI : [%s]", req->uri);

    char *userName, *wifiId, *password, *temp;

    userName = malloc(req->content_len + 1);
    wifiId = malloc(req->content_len + 1);
    password = malloc(req->content_len + 1);

	char* buf = malloc(req->content_len + 1);
	size_t off = 0;
	while (off < req->content_len) {
		int ret = httpd_req_recv(req, buf + off, req->content_len - off);
		if (ret <= 0) {
			if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
				HTTPServerErrorHandler(req);
			}
			free (buf);
			return ESP_FAIL;
		}
		off += ret;
	}
	buf[off] = '\0';
    
    temp = strtok(buf,"=");
    temp = strtok(NULL, " ");
    temp = strtok(temp, "&");
    URLDecode(userName, temp);
    
    temp = strtok(NULL, " ");
    temp = strtok(temp,"=");
    temp = strtok(NULL, " ");
    temp = strtok(temp, "&");
    URLDecode(wifiId, temp);

    temp = strtok(NULL, " ");
    temp = strtok(temp,"=");
    temp = strtok(NULL, " ");
    temp = strtok(temp, "&");
    URLDecode(password, temp);

    ESP_LOGI(TAG, "[HTTPServerPostHandler] userName : [%s], wifiId : [%s], password : [%s]", userName, wifiId, password);
    SetWiFiInformation(userName, wifiId, password);

	free(buf);
    free(userName);
    free(wifiId);
    free(password);

	httpd_resp_sendstr(req, "post successfully");
	return ESP_OK;
}

#if DEBUG
static esp_err_t DataSendEventHandler(httpd_req_t *req)
{   
    ESP_LOGI(TAG, "[DataSendEventHandler] Start");

    if(req == NULL) {
        ESP_LOGI(TAG, "[DataSendEventHandler] Server is Null");

        return ESP_FAIL;
    }
	
    ESP_LOGI(TAG, "[DataSendEventHandler] URI : [%s]", req->uri);
    MQTTPublish();

    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html>");
    httpd_resp_sendstr_chunk(req, "<head></head>");
    httpd_resp_sendstr_chunk(req, "<meta charset=""utf-8"">");
    httpd_resp_sendstr_chunk(req, "</head>");
    httpd_resp_sendstr_chunk(req, "<body>");
    
    httpd_resp_sendstr_chunk(req, "<h1>UzUs Wifi Setting</h1>");
    httpd_resp_sendstr_chunk(req, "<form method=\"post\" action=\"/post\">");
    httpd_resp_sendstr_chunk(req, "사용자 이름 <input type=\"text\" name=\"user\" value=\"");
	httpd_resp_sendstr_chunk(req, "\">");
	httpd_resp_sendstr_chunk(req, "<br>");
    httpd_resp_sendstr_chunk(req, "<br>");
	httpd_resp_sendstr_chunk(req, "WiFi 정보 <input type=\"text\" name=\"wifi\" value=\"");
	httpd_resp_sendstr_chunk(req, "\">");
	httpd_resp_sendstr_chunk(req, "<br>");
    httpd_resp_sendstr_chunk(req, "<br>");
	httpd_resp_sendstr_chunk(req, "비밀번호: <input type=\"text\" name=\"password\" value=\"");
	httpd_resp_sendstr_chunk(req, "\">");
	httpd_resp_sendstr_chunk(req, "<br>");
    httpd_resp_sendstr_chunk(req, "<br>");
	httpd_resp_sendstr_chunk(req, "<input type=\"submit\" name=\"submit\" value=\"설정\">");

	httpd_resp_sendstr_chunk(req, "</form><br>");
	httpd_resp_sendstr_chunk(req, "<hr>");

#if DEBUG
    httpd_resp_sendstr_chunk(req, "<form method=\"post\" action=\"/test\">");
	httpd_resp_sendstr_chunk(req, "<br>");
    httpd_resp_sendstr_chunk(req, "<br>");
	httpd_resp_sendstr_chunk(req, "<button onclick=\"window.location.href='/test'\">Data Send</button>");
    httpd_resp_sendstr_chunk(req, "\">");
    httpd_resp_sendstr_chunk(req, "</form><br>");
#endif

    httpd_resp_sendstr_chunk(req, "</body></html>");

    httpd_resp_sendstr_chunk(req, NULL);

    ESP_LOGI(TAG, "[DataSendEventHandler] End");

    return ESP_OK;
}
#endif

static const httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = HTTPServerRootHandler,
};

static const httpd_uri_t post = {
    .uri		 = "/post",
    .method		 = HTTP_POST,
    .handler	 = HTTPServerPostHandler,
};

#if DEBUG
static const httpd_uri_t test = {
    .uri		 = "/test",
    .method		 = HTTP_POST,
    .handler	 = DataSendEventHandler,
};
#endif

/********************************************************************************
* HTTPServerStart
* 
* Description : HTTP 서버 시작 함수로, URI 핸들러 호출
********************************************************************************/

httpd_handle_t HTTPServerStart()
{
    ESP_LOGI(TAG, "[HTTPServerStart] Start");

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "[HTTPServerStart] Port : [%d]", config.server_port);

    if(httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &post);
        httpd_register_uri_handler(server, &test);

        return server;
    }

    else {
        ESP_LOGI(TAG, "[HTTPServerStart] Server Handler is Fail");
        return NULL;
    }

    ESP_LOGI(TAG, "[HTTPServerStart] End");
}

/********************************************************************************
* HTTPServerStop
* 
* Description : HTTP 서버 종료 함수로, URI 핸들러 호출
********************************************************************************/

static void HTTPServerStop(httpd_handle_t server)
{
    ESP_LOGI(TAG, "[HTTPServerStop] Start");
    ESP_ERROR_CHECK(httpd_stop(server));
    ESP_LOGI(TAG, "[HTTPServerStop] End");
}

void HTTPServerConnectHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    ESP_LOGI(TAG, "[HTTPServerConnectHandler] Start");

    httpd_handle_t* server = (httpd_handle_t*) arg;
    if(*server == NULL) {
        ESP_LOGI(TAG, "[HTTPServerConnectHandler] Server Start");
        *server = HTTPServerStart();
    }

    else {
        ESP_LOGI(TAG, "[HTTPServerConnectHandler] Server is Busy");    
    }

    ESP_LOGI(TAG, "[HTTPServerConnectHandler] End");
}


static void HTTPServerDisConnectHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    ESP_LOGI(TAG, "[HTTPServerDisConnectHandler] Start");

    httpd_handle_t* server = (httpd_handle_t*) arg;
    if(*server) {
        ESP_LOGI(TAG, "[HTTPServerDisConnectHandler] Server Stop");
        HTTPServerStop(*server);
        *server = NULL;
    }
    
    else {
        ESP_LOGI(TAG, "[HTTPServerConnectHandler] Server is Busy");
    }

    ESP_LOGI(TAG, "[HTTPServerDisConnectHandler] End");
}
