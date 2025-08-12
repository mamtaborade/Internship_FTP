// Day 3: Basic HTTP Server (Single Client)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void get_client_ip(struct sockaddr_in *address, char *ip_buffer) {
    inet_ntop(AF_INET, &address->sin_addr, ip_buffer, INET_ADDRSTRLEN);
}

void get_current_time(char *time_buffer) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char time_buffer[26];
    char client_ip[INET_ADDRSTRLEN];
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (client_fd >= 0) {
        // Task 1 – Log the HTTP request
        read(client_fd, buffer, BUFFER_SIZE);
        printf("Received request:\n%s\n", buffer);

        // Task 2 – Get client IP address
        get_client_ip(&address, client_ip);

        // Task 2 – Get current date/time
        get_current_time(time_buffer);

        // Task 3 – Serve different responses based on URL path
        char *response_body;
        if (strstr(buffer, "GET /hello") != NULL) {
            response_body = "Hello Page";
        } else if (strstr(buffer, "GET /bye") != NULL) {
            response_body = "Goodbye Page";
        } else {
            response_body = "Default Page";
        }

        // Create the HTTP response
        snprintf(response, sizeof(response), 
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n\r\n"
                 "<html><body>"
                 "<h1>%s</h1>"
                 "<p>Current Date/Time: %s</p>"
                 "<p>Your IP Address: %s</p>"
                 "</body></html>", 
                 response_body, time_buffer, client_ip);

        write(client_fd, response, strlen(response));
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

