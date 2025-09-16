// day2_demo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    // Example PORT command string received from client
    char cmd[] = "PORT 127,0,0,1,195,80";
    int h1,h2,h3,h4,p1,p2;

    // Parse the PORT command to extract IP and port parts
    if (sscanf(cmd, "PORT %d,%d,%d,%d,%d,%d", &h1,&h2,&h3,&h4,&p1,&p2) != 6) {
        fprintf(stderr, "Failed to parse PORT command\n");
        return 1;
    }

    // Construct IP string and port number
    char ip[32];
    sprintf(ip, "%d.%d.%d.%d", h1,h2,h3,h4);
    int port = p1*256 + p2;

    printf("Client requested PORT IP=%s, port=%d\n", ip, port);

    // Create data socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &data_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address\n");
        close(sock);
        return 1;
    }

    // Connect to the client data port
    if (connect(sock, (struct sockaddr*)&data_addr, sizeof(data_addr)) == 0) {
        printf("Data connection established!\n");

        // Here, send reply "200 PORT command successful\r\n" to client
        // For demonstration, assume control connection socket is STDOUT (fd=1)
        // In real FTP server, you send this on the control connection socket
        const char *reply = "200 PORT command successful\r\n";
        ssize_t sent = write(STDOUT_FILENO, reply, strlen(reply));
        if (sent < 0) {
            perror("write");
        }
    } else {
        perror("connect");
    }

    close(sock);
    return 0;
}


