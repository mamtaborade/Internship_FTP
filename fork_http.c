// Day 4: Fork-based HTTP Server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_fd, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                      "<html><body><h1>Hello from Forked Server!</h1></body></html>";

    // Print process & client details
    time_t now = time(NULL);
    printf("Child PID: %d\n", getpid());
    printf("Client IP: %s\n", inet_ntoa(client_addr.sin_addr));
    printf("Client Port: %d\n", ntohs(client_addr.sin_port));
    printf("Connection Start Time: %s", ctime(&now));

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        // Remove trailing newline if present
        buffer[strcspn(buffer, "\r\n")] = 0;

        // Exit command from client
        if (strcasecmp(buffer, "exit") == 0) {
            printf("Client requested to close connection.\n");
            break;
        }

        // Send HTTP response
        write(client_fd, response, strlen(response));
    }

    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address, client_addr;
    socklen_t addrlen = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Connection established ...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        if (fork() == 0) {
            close(server_fd); // Child doesn't need the listening socket
            handle_client(client_fd, client_addr);
            exit(0);
        }
        close(client_fd); // Parent closes connected socket
    }

    close(server_fd);
    return 0;
}

