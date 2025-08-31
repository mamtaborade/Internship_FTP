// Day 4: Fork-based HTTP Server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include  <ctype.h>


#define PORT 8080
 
void str_tolower(char *s) {
    for (int i = 0; s[i]; i++)
        s[i] = tolower((unsigned char)s[i]);
}
void handle_client(int client_fd, struct sockaddr_in client_addr) {
    char buffer[1024];
    time_t now;
    char time_str[64];
    now = time(NULL);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(client_addr.sin_port);

    
    printf("Child PID: %d\n", getpid());
    printf("Client IP: %s\n Port: %d\n", client_ip, client_port);
    printf("Connection start time: %s\n", time_str);

    while (1) { 
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            printf("[Child PID: %d] Client disconnected\n", getpid());
            break;
        }
        char temp[2048];
        strcpy(temp, buffer);
        str_tolower(temp);
        if (strstr(temp, "exit") != NULL) {
            printf("[Child PID: %d] Exit command received, closing connection.\n", getpid());
            break;
        }  
        
        char path[64] = "/";
        sscanf(buffer, "GET %s", path);

        now = time(NULL);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

        const char *page_content;
        if (strcmp(path, "/hello") == 0)
            page_content = "<h1>Hello Page</h1>";
        else if (strcmp(path, "/bye") == 0)
            page_content = "<h1>Goodbye Page</h1>";
        else
            page_content = "<h1>Default Page</h1>";

    char response[4096];
     snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                      "<html><body><h1>Hello from Forked Server!</h1></body></html>",page_content, time_str, client_ip);

    write(client_fd, response, strlen(response));
    
}
close(client_fd);
}
int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (fork() == 0) {
            handle_client(client_fd, address);
            exit(0);
        }
        close(client_fd);
    }

    close(server_fd);
    return 0;
}