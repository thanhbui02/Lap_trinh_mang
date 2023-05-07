#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>

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

    fd_set fdread;

    int clients[64];
    char nameClients[64][50];
    int num_clients = 0;

    char buf[256];

    while (1)
    {
        // Xóa tất cả socket trong tập fdread
        FD_ZERO(&fdread);

        // Thêm socket listener vào tập fdread
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;

        // Thêm các socket client vào tập fdread
        for (int i = 0; i < num_clients; i++)
        {
            FD_SET(clients[i], &fdread);
            if (maxdp < clients[i] + 1)
                maxdp = clients[i] + 1;
        }

        // Chờ đến khi sự kiện xảy ra
        int ret = select(maxdp, &fdread, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        // Kiểm tra sự kiện có yêu cầu kết nối
        if (FD_ISSET(listener, &fdread))
        {
            char buff[50];
            char id[20];
            char name[20];
            int count=0;
            char space=' ';
            int client = accept(listener, NULL, NULL);
            printf("Có kết nối mới: %d\n", client);
            do
            {
                count=0;
                memset(name,'\0',sizeof(name));
                memset(buff,'\0',sizeof(buff));
                memset(id, '\0', sizeof(id));
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
                    if (name[i] ==space)
                    {
                        count++;
                    }
                }
            } while (strcmp(id, "client_id") != 0 || strlen(name) < 3 || count >=1);
            clients[num_clients] = client;
            strcpy(nameClients[num_clients++], name);
            printf("%s đã kết nối tới Server chat.\n", nameClients[num_clients - 1]);
        }

        // Kiểm tra sự kiện có dữ liệu truyền đến socket client
        for (int i = 0; i < num_clients; i++)
            if (FD_ISSET(clients[i], &fdread))
            {
                ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    // TODO: Client đã ngắt kết nối, xóa client ra khỏi mảng
                    continue;
                }
                buf[ret] = 0;
                // printf(" %s : %s\n", nameClients[i], buf);
                char full_msg[500];
                int msg_len = snprintf(full_msg, sizeof(full_msg), "%s:%s", nameClients[i], buf);
                full_msg[strlen(full_msg) - 1] = 0;
                for (int j = 0; j < num_clients; j++)
                {
                    if (j != i)
                    {
                        send(clients[j], full_msg, strlen(full_msg), 0);
                    }
                }
            }
    }

    close(listener);

    return 0;
}