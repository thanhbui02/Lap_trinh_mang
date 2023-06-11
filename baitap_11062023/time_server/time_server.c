#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

void formatTime(char *format, int client)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char *errmsg = "Nhập sai định dạng, mời nhập lại: ";
    bool validFormat = false;

    if (strncmp(format, "GET_TIME [dd/mm/yyyy]", strlen("GET_TIME [dd/mm/yyyy]")) == 0)
    {
        char time_str1[50];
        strftime(time_str1, sizeof(time_str1), "%d/%m/%Y", tm_info);
        send(client, time_str1, strlen(time_str1), 0);
        validFormat = true;
    }
    else if (strncmp(format, "GET_TIME [dd/mm/yy]", strlen("GET_TIME [dd/mm/yy]")) == 0)
    {
        char time_str2[50];
        strftime(time_str2, sizeof(time_str2), "%d/%m/%y", tm_info);
        send(client, time_str2, strlen(time_str2), 0);
        validFormat = true;
    }
    else if (strncmp(format, "GET_TIME [mm/dd/yyyy]", strlen("GET_TIME [mm/dd/yyyy]")) == 0)
    {
        char time_str3[50];
        strftime(time_str3, sizeof(time_str3), "%m/%d/%Y", tm_info);
        send(client, time_str3, strlen(time_str3), 0);
        validFormat = true;
    }
    else if (strncmp(format, "GET_TIME [mm/dd/yy]", strlen("GET_TIME [mm/dd/yy]")) == 0)
    {
        char time_str4[50];
        strftime(time_str4, sizeof(time_str4), "%m/%d/%y", tm_info);
        send(client, time_str4, strlen(time_str4), 0);
        validFormat = true;
    }
    // Nếu không nhập đúng định dạng
    if (!validFormat)
    {
        send(client, errmsg, strlen(errmsg), 0);
    }
}

void *handle_client(void *arg)
{
    int client = *(int *)arg;
    char buf[1024];
    char *request = "Vui lòng nhập định dạng thời gian (đúng dạng GET_TIME [format]): ";
    send(client, request, strlen(request), 0);

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            // Client đã ngắt kết nối
            break;
        }

        buf[ret] = 0;
        formatTime(buf, client);
    }

    close(client);
    pthread_exit(NULL);
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, &client) != 0)
        {
            perror("pthread_create() failed");
            close(client);
            continue;
        }
        
        pthread_detach(thread);
    }

    close(listener);
    return 0;
}
