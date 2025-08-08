#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(2121);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);
    printf("Echo server running on port 2121\n");

    int client_fd = accept(server_fd, NULL, NULL);

    
    write(client_fd, "Meenal Welcome to FTP Server\r\n", 28); 

    char buffer[1024];
    int bytes;

    while ((bytes = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
    

      buffer[bytes] = '\0';

        write(STDOUT_FILENO, buffer, bytes);
        if (strncmp(buffer, "exit", 4) == 0) { 
            printf("Client requested exit. Closing connection.\n"); 
            break; 

        
    }
       send(client_fd, buffer, bytes, 0);

    close(client_fd);
    close(server_fd);
    return 0;
}}
