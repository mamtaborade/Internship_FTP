// Day 3: Basic HTTP Server (Single Client)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<time.h>

#define PORT 8080

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct sockaddr_in address;
    char response[] = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n\r\n"
                      "<html><body><h1>Hello from Server!</h1></body></html>";
    socklen_t addrlen = sizeof(address);
    char buffer[1024];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_fd >= 0)
    {
        time_t current_time = time(NULL);
        struct tm *local_time_info = localtime(&current_time);

        char time_buffer[80];
     strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local_time_info);
     printf("Current system time: %s\n", time_buffer);

        int server_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (server_received > 0)
        {
            buffer[server_received] = '\0';
            printf("received request: %s\n", buffer);
            

            
        }
        else if
        
            (server_received == 0){

                printf("server closed connection\n");

  
            }
            else 
            {
                perror("recv failed");
            }

    char client_ip_str[INET_ADDRSTRLEN]; 
    

    if (inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip_str, sizeof(client_ip_str)) == NULL) {
        perror("inet_ntop");
      
    } else {
        printf("Client IP address: %s\n", client_ip_str);
    }

            
    }
    
    write(client_fd, response, strlen(response));
    close(client_fd);
    close(server_fd);
    return 0;
}