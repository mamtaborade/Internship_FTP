// day1_demo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 2121
#define BUFFER 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buffer[BUFFER];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 1);

    printf("FTP server running on port %d...\n", PORT);

    client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
    send(client_fd, "220 FTP Server Ready\r\n", 22, 0);

    while (1) {
        memset(buffer, 0, BUFFER);
        int n = recv(client_fd, buffer, BUFFER - 1, 0);
        if (n <= 0) break;

        printf("Client: %s", buffer);

        if (strncmp(buffer, "USER", 4) == 0) {
            send(client_fd, "331 Username OK, need password\r\n", 32, 0);
        } else if (strncmp(buffer, "PASS", 4) == 0) {
            send(client_fd, "230 Login successful\r\n", 23, 0);
        } else if (strncmp(buffer, "QUIT", 4) == 0) {
            send(client_fd, "221 Goodbye\r\n", 13, 0);
            printf("Client requested QUIT. Closing connection.\n");
            break;
        } else {
            send(client_fd, "502 Command not implemented\r\n", 29, 0);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}