#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>

void *client_thread(void *);
const char *get_content_type(const char *file_path);
void signal_handler(int signo)
{
    wait(NULL);
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

    signal(SIGPIPE, signal_handler);

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}
void send_folder(int client, const char *folder_path)
{
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char buffer[2048];
    char current_dir[256];

    // Lưu đường dẫn hiện tại
    getcwd(current_dir, sizeof(current_dir));

    // Thay đổi thư mục hiện tại thành thư mục được chỉ định
    if (chdir(folder_path) == -1) {
        char *response_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
        send(client, response_header, strlen(response_header), 0);
        char *response_body = "<html><body><h1>Folder Not Found</h1></body></html>";
        send(client, response_body, strlen(response_body), 0);
        return;
    }

    dir = opendir(".");
    if (dir == NULL)
    {
        char *response_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
        send(client, response_header, strlen(response_header), 0);
        char *response_body = "<html><body><h1>Folder Not Found</h1></body></html>";
        send(client, response_body, strlen(response_body), 0);

        // Phục hồi thư mục hiện tại
        chdir(current_dir);

        return;
    }

    char response_header[2048];
    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    send(client, response_header, strlen(response_header), 0);

    char *response_body = "<html><body>";
    send(client, response_body, strlen(response_body), 0);

    // Đường dẫn thư mục con
    char subfolder_path[1024];

    while ((entry = readdir(dir)) != NULL)
    {
        // Đường dẫn thư mục con hoặc tệp tin
        sprintf(subfolder_path, "%s/%s", folder_path, entry->d_name);
        stat(subfolder_path, &file_stat);

        if (S_ISDIR(file_stat.st_mode))
        {
            // Directory
            sprintf(buffer, "<p><a href=\"%s/\">%s/</a></p>", entry->d_name, entry->d_name);
            send(client, buffer, strlen(buffer), 0);
        }
        else if (S_ISREG(file_stat.st_mode))
        {
            // File
            sprintf(buffer, "<p><a href=\"%s\">%s</a></p>", entry->d_name, entry->d_name);
            send(client, buffer, strlen(buffer), 0);
        }
    }

    closedir(dir);

    char *response_footer = "</body></html>";
    send(client, response_footer, strlen(response_footer), 0);

    // Phục hồi thư mục hiện tại
    chdir(current_dir);
}




void send_file(int client, const char *file_path)
{
    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        char *response_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
        send(client, response_header, strlen(response_header), 0);
        char *response_body = "<html><body><h1>File Not Found</h1></body></html>";
        send(client, response_body, strlen(response_body), 0);
        return;
    }

    char response_header[2048];
    sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", get_content_type(file_path));
    send(client, response_header, strlen(response_header), 0);

    char buffer[2048];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        send(client, buffer, bytesRead, 0);
    }

    fclose(file);
}

const char *get_content_type(const char *file_path)
{
    const char *extension = strrchr(file_path, '.');
    if (extension != NULL)
    {
        if (strcmp(extension, ".txt") == 0)
            return "text/plain";
        else if (strcmp(extension, ".c") == 0)
            return "text/plain";
        else if (strcmp(extension, ".cpp") == 0)
            return "text/plain";
        else if (strcmp(extension, ".jpg") == 0)
            return "image/jpeg";
        else if (strcmp(extension, ".png") == 0)
            return "image/png";
        else if (strcmp(extension, ".mp3") == 0)
            return "audio/mp3";
    }
    return "application/octet-stream";
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[2048];

    int ret = recv(client, buf, sizeof(buf) - 1, 0);
    if (ret <= 0)
        return NULL;

    buf[ret] = 0;
    printf("Received from %d: %s\n", client, buf);

    char method[16];
    char path[256];
    sscanf(buf, "%s %s", method, path);

    if (strcmp(path, "/") == 0)
    {
        char *response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        send(client, response_header, strlen(response_header), 0);

        char *response_body = "<html><body>";
        send(client, response_body, strlen(response_body), 0);

        DIR *dir;
        struct dirent *entry;
        dir = opendir(".");
        if (dir != NULL)
        {
            while ((entry = readdir(dir)) != NULL)
            {
                struct stat file_stat;
                stat(entry->d_name, &file_stat);
                if (S_ISDIR(file_stat.st_mode))
                {
                    // Directory
                    sprintf(buf, "<p><a href=\"%s/\">%s/</a></p>", entry->d_name, entry->d_name);
                    send(client, buf, strlen(buf), 0);
                }
                else
                {
                    // File
                    sprintf(buf, "<p><a href=\"%s\">%s</a></p>", entry->d_name, entry->d_name);
                    send(client, buf, strlen(buf), 0);
                }
            }
            closedir(dir);
        }

        char *response_footer = "</body></html>";
        send(client, response_footer, strlen(response_footer), 0);
    }
    else
    {
        // Combine the current directory path and the requested file name
        char current_dir[256];
        getcwd(current_dir, sizeof(current_dir));
        char file_path[1024];
        sprintf(file_path, "%s/%s", current_dir, path);

        struct stat file_stat;
        if (stat(file_path, &file_stat) == 0)
        {
            if (S_ISDIR(file_stat.st_mode))
            {
                // Requested path is a directory
                send_folder(client, file_path);
            }
            else if (S_ISREG(file_stat.st_mode))
            {
                // Requested path is a regular file
                send_file(client, file_path);
            }
        }
        else
        {
            // File or directory not found
            char *response_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
            send(client, response_header, strlen(response_header), 0);
            char *response_body = "<html><body><h1>File or Folder Not Found</h1></body></html>";
            send(client, response_body, strlen(response_body), 0);
        }
    }

    close(client);
}
