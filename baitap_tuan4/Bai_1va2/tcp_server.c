#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("Usage: %s <port> <welcome_file> <output_file>\n", argv[0]);
        return 1;
    }

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    int portNum = atoi(argv[1]);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(portNum);

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

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    FILE *welcome;
    welcome = fopen(argv[2], "r");
    if (welcome == NULL) {
        perror("Error opening welcome file");
        return 1;
    }
    char hello[256];
    fgets(hello, 256, welcome);
    send(client, hello, strlen(hello), 0);
    fclose(welcome);

    char buf[256];
    FILE *output;
    output = fopen(argv[3], "w");
    if (output == NULL) {
        perror("Error opening output file");
        return 1;
    }
    while (1)
    {
        memset(buf,'\0',256);
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            printf("Ket noi bi dong.\n");
            break;
        }
        fflush(stdin);
        fprintf(output, "%s", buf);
        printf("Tin nhan duoc them vao file output:%s",buf);
    }

    fclose(output);
    close(client);
    close(listener);

    return 0;
}
