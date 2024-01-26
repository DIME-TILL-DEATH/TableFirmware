#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "tcpip.hpp"

#include "frameparser.hpp"
#include "netcomm/abstractcommand.hpp"

#include "projdefines.h"

#include "netanswer.hpp"

#define PORT                        3333
#define KEEPALIVE_IDLE              5
#define KEEPALIVE_INTERVAL          5
#define KEEPALIVE_COUNT             3

// TODO: for multiple connections
 #define MAX_CONNECTIONS 4
// FrameParser* frameParser[MAX_CONNECTIONS];

static const char *TAG = "TCP/IP";

TaskHandle_t answerTaskHandle;

static void socket_recv_task(const int sock)
{
    int len;
    uint8_t rx_buffer[512];
    FrameParser* frameParser = new FrameParser(sock);

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) 
        {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } 
        else if (len == 0) 
        {
            ESP_LOGW(TAG, "Connection closed");
        } 
        else 
        {    
            //ESP_LOGI(TAG, "Recieved frame");        
            frameParser->processRecvData(rx_buffer, len);           
                        
            while(frameParser->parsedCommands.size()>0)
            {
                NetComm::AbstractCommand* recvComm = frameParser->parsedCommands.front();
                frameParser->parsedCommands.erase(frameParser->parsedCommands.begin());
                switch(recvComm->commandType())
                {
                    case NetComm::TRANSPORT_COMMAND: 
                    {
                        xQueueSendToBack(printReqQueue, &recvComm, pdMS_TO_TICKS(1000)); 
                        break;
                    }
                    case NetComm::PLAYLIST_COMMAND: 
                    {
                        xQueueSendToBack(fileReqQueue, &recvComm, pdMS_TO_TICKS(1000)); 
                        break;
                    }
                    case NetComm::FILE_COMMAND:
                    {
                        xQueueSendToBack(netAnswQueue, &recvComm, pdMS_TO_TICKS(10000));
                        break;
                    }
                    case NetComm::FIRMWARE_COMMAND:
                    {
                        xQueueSendToBack(netAnswQueue, &recvComm, pdMS_TO_TICKS(10000));
                        break;
                    }
                    default: ESP_LOGE(TAG, "Unknown command type"); break;
                }
            }
        }
    } while (len > 0);

// TODO handle WI-FI disconnect. Close socket and delete tasks
    delete(frameParser);
    shutdown(sock, 0);
    close(sock);
    ESP_LOGI(TAG, "Frame parser deleted, socket closed");
}

static void answer_task(void *pvParameters)
{
    NetComm::AbstractCommand* recvComm;
    int socket = *((int*)pvParameters);

    for(;;)
    {
        portBASE_TYPE xStatus = xQueueReceive(netAnswQueue, &recvComm, pdMS_TO_TICKS(10));
        if(xStatus == pdPASS)
        {

            processAnswer(recvComm, socket);
            if(recvComm) delete recvComm;         
        }
    }
}

static void tcp_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = AF_INET;
    
    struct sockaddr_storage dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);

    int listen_sock = socket(addr_family, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) 
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) 
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, MAX_CONNECTIONS);
    if (err != 0) 
    {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) 
    {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);

        if (sock < 0) 
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        int keepAlive = 1;
        int keepIdle = KEEPALIVE_IDLE;
        int keepInterval = KEEPALIVE_INTERVAL;
        int keepCount = KEEPALIVE_COUNT;
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));

        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) 
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        xTaskCreatePinnedToCore(answer_task, "tcp_answer", 8192, (void*)&sock, PRIORITY_TCP_ANSWER_TASK, &answerTaskHandle, 0);
        socket_recv_task(sock);     
        vTaskDelete(answerTaskHandle);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);   
}

void TCPIP_Init(void)
{
    xTaskCreatePinnedToCore(tcp_task, "tcp_server", 8192, NULL, PRIORITY_TCP_RECIEVE_TASK, NULL, 0);
}