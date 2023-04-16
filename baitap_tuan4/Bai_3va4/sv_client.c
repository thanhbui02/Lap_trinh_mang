#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 256
struct sinhvien
{
    char MSSV[20];
    char HoTen[50];
    int NgaySinh;
    int ThangSinh;
    int NamSinh;
    float DiemTBC;
};

int main(int argc, char *argv[])
{
    // Kiểm tra tham số đầu vào
    if (argc != 3)
    {
        printf("Sử dụng: %s <địa chỉ IP> <cổng>\n", argv[0]);
        exit(1);
    }

    // Tạo socket
    int client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_sock == -1)
    {
        perror("socket() failed");
        exit(1);
    }

    // Thiết lập địa chỉ và cổng của server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Kết nối đến server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect() failed");
        exit(1);
    }

    // Nhập dữ liệu từ bàn phím và gửi đến server
    while (1)
    {
        fflush(stdin);
        struct sinhvien sinhvien;
        printf("\n\n**Nhập thông tin sinh viên, nếu muốn thoát ấn tổ hợp Ctrl + C**\n\n");
        fflush(stdin);

        printf("Nhap MSSV: ");
        fflush(stdin);
        scanf("%s", sinhvien.MSSV);

        printf("Nhap ho ten: ");
        scanf(" %[^\n]", sinhvien.HoTen);

        printf("Nhap ngay sinh: ");
        fflush(stdin);
        scanf("%d", &sinhvien.NgaySinh);

        printf("Nhap thang sinh: ");
        fflush(stdin);
        scanf("%d", &sinhvien.ThangSinh);

        printf("Nhap nam sinh: ");
        fflush(stdin);
        scanf("%d", &sinhvien.NamSinh);

        printf("Nhap diem trung binh: ");
        fflush(stdin);
        scanf("%f", &sinhvien.DiemTBC);
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char s[20];
        strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", tm);
        char message[256];
        sprintf(message, "%s %s %s %d-%02d-%02d %.2f",s, sinhvien.MSSV, sinhvien.HoTen, sinhvien.NamSinh, sinhvien.ThangSinh, sinhvien.NgaySinh, sinhvien.DiemTBC);
        printf("%s",message);
        send(client_sock,message,strlen(message),0);
    }

    // Đóng kết nối
    close(client_sock);
    return 0;
}
