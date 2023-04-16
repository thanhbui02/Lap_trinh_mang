#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<time.h>
int main(int argc, char *argv[])
{
    if (argc != 3) {
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


    char buf[256];
    FILE *log_file;
    log_file = fopen(argv[2], "w");
    if (log_file == NULL) {
        perror("Error opening output file");
        return 1;
    }
    // while (1)
    // {

    //     int str_len = recv(client, buf, sizeof(buf), 0);
    //     if (str_len <= 0 )
    //     {
    //         printf("Disconnect!\n");
    //         return 0;
    //     }

    //     time_t t = time(NULL);
    //     struct tm *tm = localtime(&t);
    //     char s[20];
    //     strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", tm);
    //     char msg[1000];
    //     sprintf(msg, "%s %s %s\n",inet_ntoa(clientAddr.sin_addr),s,buf );
    //     fwrite(msg,1,strlen(msg),log_file);
    //     printf("%s\n",msg);
    // }
while (1)
    {
        memset(buf,'\0',256);
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            printf("Ket noi bi dong.\n");
            break;
        }
         char msg[1000];
        sprintf(msg, "%s %s\n",inet_ntoa(clientAddr.sin_addr),buf );
        fwrite(msg,1,strlen(msg),log_file);
        printf("%s\n",msg);
    }
    fclose(log_file);
    close(client);
    close(listener);

    return 0;
}
