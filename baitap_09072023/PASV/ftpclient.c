#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(21);

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    // Nhan xau chao
    int len = recv(client, buf, sizeof(buf), 0);
    buf[len] = 0;

    puts(buf);

    char username[64], password[64];

    while (1)
    {
        printf("Nhap username: ");
        scanf("%s", username);
        printf("Nhap password: ");
        scanf("%s", password);

        // Gui lenh USER
        sprintf(buf, "USER %s\r\n", username);
        send(client, buf, strlen(buf), 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        // puts(buf);

        // Gui lenh PASS
        sprintf(buf, "PASS %s\r\n", password);
        send(client, buf, strlen(buf), 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        // puts(buf);

        if (strncmp(buf, "230", 3) == 0)
        {
            printf("Dang nhap thanh cong.\n");
            break;
        }
        else
        {
            printf("Dang nhap that bai. Hay nhap lai.\n");
        }
    }

    // Lay noi dung cua thu muc truoc khi upload
    {
        // Gui lenh PASV
        send(client, "PASV\r\n", 6, 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Xac dinh dia chi IP va cong cua kenh du lieu
        char *pos1 = strchr(buf, '(') + 1;
        char *pos2 = strchr(pos1, ')');
        *pos2 = 0;

        unsigned int ip[4], port[2];
        sscanf(pos1, "%u,%u,%u,%u,%u,%u", &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);

        char data_ip[16];
        sprintf(data_ip, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

        int data_port = port[0] * 256 + port[1];

        // Mo ket noi moi den kenh du lieu
        int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        data_addr.sin_port = htons(data_port);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)))
        {
            perror("connect() failed");
            return 1;
        }

        // Gui lenh LIST
        send(client, "LIST\r\n", 6, 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        printf("Truoc khi upload:   ");
        puts(buf);

        // Nhan du lieu o kenh du lieu
        while (1)
        {
            len = recv(data_socket, buf, sizeof(buf), 0);
            if (len <= 0)
                break;
            buf[len] = 0;
            puts(buf);
        }

        close(data_socket);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);
    }

    // Upload file
    {
        // Gui lenh PASV
        send(client, "PASV\r\n", 6, 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Xac dinh dia chi IP va cong cua kenh du lieu
        char *pos1 = strchr(buf, '(') + 1;
        char *pos2 = strchr(pos1, ')');
        *pos2 = 0;

        unsigned int ip[4], port[2];
        sscanf(pos1, "%u,%u,%u,%u,%u,%u", &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);

        char data_ip[16];
        sprintf(data_ip, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

        int data_port = port[0] * 256 + port[1];

        // Mo ket noi moi den kenh du lieu
        int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        data_addr.sin_port = htons(data_port);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)))
        {
            perror("connect() failed");
            return 1;
        }

        // Gui lenh STOR ten_file
        FILE *f = fopen("fileupload.txt", "rb");
        if (!f)
        {
            perror("fopen() failed");
            return 1;
        }

        sprintf(buf, "STOR fileupload.txt\r\n");
        send(client, buf, strlen(buf), 0);

        // Phan hoi so 1 cua lenh STOR
        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Nhan du lieu o kenh du lieu
        while (1)
        {
            len = fread(buf, 1, sizeof(buf), f);
            if (len <= 0)
                break;
            send(data_socket, buf, len, 0);
        }

        fclose(f);
        close(data_socket);

        // Phan hoi so 2 cua lenh STOR
        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);
    }

    // Lay noi dung cua thu muc sau khi upload
    {
        // Gui lenh PASV
        send(client, "PASV\r\n", 6, 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);

        // Xac dinh dia chi IP va cong cua kenh du lieu
        char *pos1 = strchr(buf, '(') + 1;
        char *pos2 = strchr(pos1, ')');
        *pos2 = 0;

        unsigned int ip[4], port[2];
        sscanf(pos1, "%u,%u,%u,%u,%u,%u", &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);

        char data_ip[16];
        sprintf(data_ip, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);

        int data_port = port[0] * 256 + port[1];

        // Mo ket noi moi den kenh du lieu
        int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        data_addr.sin_port = htons(data_port);

        if (connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr)))
        {
            perror("connect() failed");
            return 1;
        }

        // Gui lenh LIST
        send(client, "LIST\r\n", 6, 0);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        printf("Sau khi upload:   ");
        puts(buf);

        // Nhan du lieu o kenh du lieu
        while (1)
        {
            len = recv(data_socket, buf, sizeof(buf), 0);
            if (len <= 0)
                break;
            buf[len] = 0;
            puts(buf);
        }

        close(data_socket);

        len = recv(client, buf, sizeof(buf), 0);
        buf[len] = 0;
        puts(buf);
    }

    // Gui lenh QUIT
    send(client, "QUIT\r\n", 6, 0);

    // Ket thuc, dong socket
    close(client);

    return 0;
}