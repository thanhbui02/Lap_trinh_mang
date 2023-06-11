#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

#define MAX_CLIENTS 64
#define BUFFER_SIZE 256

int clients[MAX_CLIENTS];
char nameClients[MAX_CLIENTS][50];
int num_clients = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg)
{
    int client = *(int *)arg;
    char buf[BUFFER_SIZE];

    char id[20];
    char name[20];
    int count = 0;
    char space = ' ';

    do
    {
        char request[] = "Vui lòng nhập tên của bạn (đúng định dạng client_id: name):";
        pthread_mutex_lock(&mutex); // Khóa mutex trước khi gửi yêu cầu đăng nhập
        int s = send(client, request, strlen(request), 0);
        pthread_mutex_unlock(&mutex); // Mở khóa mutex sau khi gửi yêu cầu đăng nhập

        memset(buf, 0, sizeof(buf));
        memset(id, 0, sizeof(id));
        memset(name, 0, sizeof(name));
        int rcv = recv(client, buf, sizeof(buf), 0);
        if (rcv <= 0)
        {
            break;
        }

        if (strchr(buf, ':') == NULL)
            continue;

        char *token = strtok(buf, ":");
        strcpy(id, token);
        token = strtok(NULL, ":");
        token[strlen(token) - 1] = 0;
        strcpy(name, token);
        for (int i = 1; i < strlen(name); i++)
        {
            if (name[i] == space)
            {
                count++;
            }
        }

    } while (strcmp(id, "client_id") != 0 || strlen(name) < 3 || count >= 1);

    pthread_mutex_lock(&mutex); // Khóa mutex trước khi thêm client mới
    clients[num_clients] = client;
    strcpy(nameClients[num_clients], name);
    printf("%s đã kết nối tới Server chat.\n", nameClients[num_clients]);
    num_clients++;
    pthread_mutex_unlock(&mutex); // Mở khóa mutex sau khi thêm client mới

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            // Client đã ngắt kết nối
            break;
        }
        buf[ret] = 0;

        char full_msg[500];
        int msg_len = snprintf(full_msg, sizeof(full_msg), "%s: %s", name, buf);
        full_msg[msg_len] = 0;

        pthread_mutex_lock(&mutex); // Khóa mutex trước khi gửi tin nhắn cho các client khác
        for (int i = 0; i < num_clients; i++)
        {
            if (clients[i] != client)
            {
                send(clients[i], full_msg, strlen(full_msg), 0);
            }
        }
        pthread_mutex_unlock(&mutex); // Mở khóa mutex sau khi gửi tin nhắn cho các client khác
    }

    pthread_mutex_lock(&mutex); // Khóa mutex trước khi loại bỏ client đã ngắt khỏi danh sách
    for (int i = 0; i < num_clients; i++)
    {
        if (clients[i] == client)
        {
            // Dịch chuyển các phần tử trong mảng để loại bỏ client
            for (int j = i; j < num_clients - 1; j++)
            {
                clients[j] = clients[j + 1];
                strcpy(nameClients[j], nameClients[j + 1]);
            }
            break;
        }
    }
    num_clients--;
    pthread_mutex_unlock(&mutex); // Mở khóa mutex sau khi loại bỏ client đã ngắt khỏi danh sách

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
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        int max_fd = listener;

        pthread_mutex_lock(&mutex); // Khóa mutex trước khi thao tác trên danh sách client
        for (int i = 0; i < num_clients; i++)
        {
            int client_fd = clients[i];
            FD_SET(client_fd, &readfds);
            if (client_fd > max_fd)
                max_fd = client_fd;
        }
        pthread_mutex_unlock(&mutex); // Mở khóa mutex sau khi thao tác trên danh sách client

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0)
        {
            perror("select() failed");
            continue;
        }

        if (FD_ISSET(listener, &readfds))
        {
            int client = accept(listener, NULL, NULL);
            if (client == -1)
            {
                perror("accept() failed");
                continue;
            }

            if (num_clients >= MAX_CLIENTS)
            {
                printf("Số lượng kết nối đã đạt tối đa. Từ chối kết nối mới.\n");
                close(client);
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
    }

    close(listener);
    return 0;
}
