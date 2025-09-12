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

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Setup address struct
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("FTP server running on port %d...\n", PORT);
    fflush(stdout);

    // Accept client connection
    client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
    if (client_fd < 0) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Send FTP greeting
    const char *greeting = "220 FTP Server Ready\r\n";
    send(client_fd, greeting, strlen(greeting), 0);

    while (1) {
        memset(buffer, 0, BUFFER);
        int n = recv(client_fd, buffer, BUFFER - 1, 0);
        if (n <= 0) break;

        printf("Client: %s", buffer);
        fflush(stdout);

        if (strncmp(buffer, "USER", 4) == 0) {
            const char *user_resp = "331 Username OK, need password\r\n";
            send(client_fd, user_resp, strlen(user_resp), 0);
        } else if (strncmp(buffer, "PASS", 4) == 0) {
            const char *pass_resp = "230 Login successful\r\n";
            send(client_fd, pass_resp, strlen(pass_resp), 0);
        } else if (strncmp(buffer, "QUIT", 4) == 0) {
            const char *quit_resp = "221 Goodbye\r\n";
            send(client_fd, quit_resp, strlen(quit_resp), 0);
            break;
        } else {
            const char *unknown_resp = "502 Command not implemented\r\n";
            send(client_fd, unknown_resp, strlen(unknown_resp), 0);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}

