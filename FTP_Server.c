#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_BUFFER 1024

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
    char buffer[1024];
    int bytes;
    
        // Task(day04): Send welcome message to client
    char *welcome_msg = "Welcome to the TCP server!\n";
    write(client_fd, welcome_msg, strlen(welcome_msg));  // send to client

    // Print on server terminal
    printf("telnet connected. Sent welcome message.\n");

 while ((bytes = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        // Print the received message on the server terminal
        printf("Received message: %.*s\n", bytes, buffer);
        
        //Task(day05): Check if the received message is "exit"
        if (strncmp(buffer, "exit" , 4) == 0){
        	printf("Client requested to exit. connection closed\n");
        	break;
        }
        
        send(client_fd, buffer, bytes, 0);
    }
    close(client_fd);
    close(server_fd);
    return 0;
}
