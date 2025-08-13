// Day 4: Fork-based HTTP Server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080

void handle_client(int client_fd, struct sockaddr_in client_address) {
    // Task1 Get the client's IP address and port number
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(client_address.sin_port);

    // Task1 Get the current time
    time_t current_time;
    time(&current_time);
    char* time_str = ctime(&current_time);
    time_str[strlen(time_str) - 1] = '\0'; // Remove the newline character

    // Task1 Print process details
    printf("Child process PID: %d\n", getpid());
    printf("Client IP: %s, Port: %d\n", client_ip, client_port);
    printf("Connection start time: %s\n", time_str);

    // Send HTTP response
    char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                      "<html><body><h1>Hello from Forked Server!</h1></body></html>";
    write(client_fd, response, strlen(response));
    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address, client_address;
    socklen_t addrlen = sizeof(client_address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_address, &addrlen);
        if (fork() == 0) {
            handle_client(client_fd, client_address);
            exit(0);
        }
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

