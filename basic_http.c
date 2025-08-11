// Day 3: Basic HTTP Server (Single Client)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    char response[] = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n\r\n"
                      "<html><body><h1>Hello from Server!</h1></body></html>";
    char request[BUFFER_SIZE];
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (client_fd >= 0) {
        // TASK: Read the HTTP request
        ssize_t bytes_read = read(client_fd, request, sizeof(request) - 1);
        if (bytes_read > 0) {
            request[bytes_read] = '\0'; // Null-terminate the string
            printf("Received HTTP request:\n%s\n", request); // Log the request
        }

        // Send the response
        write(client_fd, response, strlen(response));
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

