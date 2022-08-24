#ifndef __FILE_H__
#define __FILE_H__

#include "esp_http_server.h"

#define SPIFFS_USER_NAME_PATH   "/spiffs/userName.txt"
#define SPIFFS_WIFI_PATH        "/spiffs/wifi.txt"
#define SPIFFS_PASSWORD_PATH    "/spiffs/password.txt"
#define SPIFFS_FILE_WRITE       "w"
#define SPIFFS_FILE_READ        "r"

#define FILE_PATH_MAX_SIZE        256

char* FileRead(char *filePath, char* fileMode);
void FileWrite(char *fileInfo, char *filePath, char* fileMode);

void URLDecode(char *dst, const char *src);

#endif