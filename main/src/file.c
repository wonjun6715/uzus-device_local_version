#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "os.h"

#include "../inc/file.h"

static const char* TAG = "FILE";

/********************************************************************************
* FileWrite
* Parameter 
*   fileInfo : File에 Write할 내용
*   filePath : SPIFFS Path 정보
*   fileMode : 읽기 / 쓰기 모드
* 
* Description : File에 WiFi, Password, User 정보를 Write하기 위한 함수
********************************************************************************/

void FileWrite(char *fileInfo, char *filePath, char* fileMode)
{
    FILE* f = fopen(filePath, fileMode);
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    fprintf(f, fileInfo);
    fclose(f);
    ESP_LOGI(TAG, "File written");
}

/********************************************************************************
* FileRead
* Parameter 
*   filePath : SPIFFS Path 정보
*   fileMode : 읽기 / 쓰기 모드
* 
* Description : File에 WiFi, Password, User 정보를 Read 하기 위한 함수
********************************************************************************/

char* FileRead(char *filePath, char* fileMode)
{
    FILE* f;

    f = fopen(filePath, fileMode);
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    
    char line[64];
    fgets(line, sizeof(line), f);
    
    fclose(f);

    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return line;
}

/********************************************************************************
* URLDecode
* Parameter 
*   inText : URL Encode String
*   outText : 출력 String
* 
* Description : User, WiFi, Password 정보를 Parsing 할 때 Decode 하는 함수
********************************************************************************/

void URLDecode(char *outText, const char *inText)
{
    char a, b;
    while (*inText) {
        if ((*inText == '%') && ((a = inText[1]) && (b = inText[2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') {
                a -= 'a'-'A';
            }
            if (a >= 'A') {
                a -= ('A' - 10);
            }
            else {
                a -= '0';
            }
            
            if (b >= 'a') {
                b -= 'a'-'A';
            }
                    
            if (b >= 'A') {
                b -= ('A' - 10);
            }
                    
            else {
                b -= '0';
            }
                    
            *outText++ = 16*a+b;
            inText+=3;
        } 

        else if (*inText == '+') {
            *outText++ = ' ';
            inText++;
        } 
        
        else {
            *outText++ = *inText++;
        }
    }

    *outText++ = '\0';
}