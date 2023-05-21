#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h> // Include poll.h for using the poll function

#define MAX_CLIENTS 64

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

    struct pollfd fds[MAX_CLIENTS + 1]; // Define the pollfd array (+1 for the listener)
    int nfds = 1;                       // Number of file descriptors, starting from 1 with the listener

    fds[0].fd = listener;
    fds[0].events = POLLIN; // Event for read (data available)

    int clients[MAX_CLIENTS];
    char nameClients[MAX_CLIENTS][50];
    int num_clients = 0;

    char buf[256];

    while (1)
    {
        int ret = poll(fds, nfds, -1); // -1 to wait indefinitely

        if (ret < 0)
        {
            perror("poll() failed");
            return 1;
        }

        // Check for connection request event
        if (fds[0].revents & POLLIN)
        {
            char buff[50];
            char id[20];
            char name[20];
            int count = 0;
            char space = ' ';
            memset(buff, 0, sizeof(buff));
            memset(id, 0, sizeof(id));
            memset(name, 0, sizeof(name));
            int client = accept(listener, NULL, NULL);
            printf("Có kết nối mới: %d\n", client);
            do
            {
                count = 0;
                memset(name, '\0', sizeof(name));
                memset(buff, '\0', sizeof(buff));
                int s = send(client, "Vui lòng nhập tên của bạn (đúng định dạng client_id: name):", strlen("Vui lòng nhập tên của bạn (đúng định dạng client_id:name):"), 0);
                if (s <= 0)
                {
                    break;
                }
                int rcv = recv(client, buff, sizeof(buff), 0);
                if (rcv <= 0)
                {
                    break;
                }

                if (strchr(buff, ':') == NULL)
                    continue;

                char *token = strtok(buff, ":");
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

            if (num_clients >= MAX_CLIENTS)
            {
                printf("Số lượng kết nối đã đạt tối đa. Từ chối kết nối mới.\n");
                close(client);
            }
            else
            {
                clients[num_clients] = client;
                strcpy(nameClients[num_clients], name);
                printf("%s đã kết nối tới Server chat.\n", nameClients[num_clients]);
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
                num_clients++;
            }
        }

        // Check for data received event from client sockets
        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    // Client đã ngắt kết nối, xóa client ra khỏi mảng và giảm số lượng nfds
                    printf("%s đã ngắt kết nối.\n", nameClients[i - 1]);
                    close(fds[i].fd);
                    for (int j = i; j < nfds - 1; j++)
                    {
                        fds[j] = fds[j + 1];
                    }
                    nfds--;
                    i--; // Giảm chỉ số i để kiểm tra socket mới đến
                    continue;
                }
                buf[ret] = 0;
                char full_msg[500];
                int msg_len = snprintf(full_msg, sizeof(full_msg), "%s:%s", nameClients[i - 1], buf);
                full_msg[strlen(full_msg) - 1] = 0;
                for (int j = 1; j < nfds; j++)
                {
                    if (j != i)
                    {
                        send(fds[j].fd, full_msg, strlen(full_msg), 0);
                    }
                }
            }
        }
    }

    close(listener);

    return 0;
}
