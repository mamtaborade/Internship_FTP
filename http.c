
// Day 3: Basic HTTP Server (Single Client)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    char buffer [4096] ;
    char response[] = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html\r\n\r\n"
                      "<html><body><h1>Hello from Server!</h1></body></html>";
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    if (client_fd >= 0) {

        memset(buffer,0,sizeof(buffer)); // read the http request and first empty the buffer 


        read(client_fd,buffer,sizeof(buffer)-1); // read data from client 
        printf("Raw HTTP Request Recived :\n...................\n");
        printf("%s\n",buffer);
        write(client_fd, response, strlen(response));
        close(client_fd);
    }

    close(server_fd);
    return 0;
}