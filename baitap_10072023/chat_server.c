#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>

#define MAX_CLIENTS 10
#define MAX_TOPIC_LENGTH 100
#define MAX_MESSAGE_LENGTH 200

typedef struct {
    int socket;
    char nickname[20];
    int operator;
} Client;

typedef struct {
    char topic[MAX_TOPIC_LENGTH];
    Client* clients[MAX_CLIENTS];
    int clientCount;
} ChatRoom;

ChatRoom chatRoom;

void initializeChatRoom() {
    strcpy(chatRoom.topic, "");
    chatRoom.clientCount = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        chatRoom.clients[i] = NULL;
    }
}

void broadcastMessage(char* message) {
    for (int i = 0; i < chatRoom.clientCount; i++) {
        Client* client = chatRoom.clients[i];
        send(client->socket, message, strlen(message), 0);
    }
}

void processCommand(char* command, int clientSocket) {
    char* token = strtok(command, " ");
    if (strcmp(token, "TOPIC") == 0) {
        token = strtok(NULL, "");
        strcpy(chatRoom.topic, token);
        send(clientSocket, "100 OK\n", 7, 0);
    } else if (strcmp(token, "JOIN") == 0) {
        token = strtok(NULL, " ");
        Client* client = malloc(sizeof(Client));
        client->socket = clientSocket;
        strcpy(client->nickname, token);
        client->operator = 0;
        chatRoom.clients[chatRoom.clientCount++] = client;
        send(clientSocket, "100 OK\n", 7, 0);
    } else if (strcmp(token, "OP") == 0) {
        token = strtok(NULL, "");
        for (int i = 0; i < chatRoom.clientCount; i++) {
            Client* client = chatRoom.clients[i];
            if (strcmp(client->nickname, token) == 0) {
                client->operator = 1;
                break;
            }
        }
        send(clientSocket, "100 OK\n", 7, 0);
    } else if (strcmp(token, "MSG") == 0) {
        token = strtok(NULL, "");
        char message[MAX_MESSAGE_LENGTH + 6];
        sprintf(message, "MSG %s\n", token);
        broadcastMessage(message);
        send(clientSocket, "100 OK\n", 7, 0);
    } else if (strcmp(token, "PMSG") == 0) {
        token = strtok(NULL, " ");
        char* recipient = token;
        token = strtok(NULL, "");
        char message[MAX_MESSAGE_LENGTH + 8];
        sprintf(message, "PMSG %s %s\n", recipient, token);
        for (int i = 0; i < chatRoom.clientCount; i++) {
            Client* client = chatRoom.clients[i];
            if (strcmp(client->nickname, recipient) == 0) {
                send(client->socket, message, strlen(message), 0);
                break;
            }
        }
        send(clientSocket, "100 OK\n", 7, 0);
    } else if (strcmp(token, "KICK") == 0) {
        token = strtok(NULL, "");
        for (int i = 0; i < chatRoom.clientCount; i++) {
            Client* client = chatRoom.clients[i];
            if (strcmp(client->nickname, token) == 0) {
                char kickMessage[15];
                sprintf(kickMessage, "KICK %s\n", token);
                send(client->socket, kickMessage, strlen(kickMessage), 0);
                close(client->socket);
                chatRoom.clientCount--;
                free(client);
                for (int j = i; j < chatRoom.clientCount; j++) {
                    chatRoom.clients[j] = chatRoom.clients[j + 1];
                }
                break;
            }
        }
        send(clientSocket, "100 OK\n", 7, 0);
    } else if (strcmp(token, "QUIT") == 0) {
        send(clientSocket, "100 OK\n", 7, 0);
        close(clientSocket);
    }
}

void handleClient(int clientSocket) {
    char buffer[MAX_MESSAGE_LENGTH];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    buffer[bytesRead] = '\0';
    processCommand(buffer, clientSocket);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(struct sockaddr_in);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8080);

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    listen(serverSocket, 10);

    initializeChatRoom();

    printf("Server listening on port 8080...\n");

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
        printf("Client connected\n");
        handleClient(clientSocket);
    }

    return 0;
}
