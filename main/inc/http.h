#ifndef __HTTP_H__
#define __HTTP_H__

#include "esp_event.h"
#include "common.h"

void HTTPServerConnectHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
static void HTTPServerDisConnectHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);
httpd_handle_t HTTPServerStart();
static void HTTPServerStop(httpd_handle_t server);
esp_err_t HTTPServerErrorHandler(httpd_req_t* req);
static esp_err_t HTTPServerRootHandler(httpd_req_t *req);
static esp_err_t HTTPServerPostHandler(httpd_req_t *req);

#if DEBUG
static esp_err_t DataSendEventHandler(httpd_req_t *req);
#endif

#endif