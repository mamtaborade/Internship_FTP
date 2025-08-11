
// Day 3: Basic HTTP Server (Single Client)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>



#define PORT 8080

void newline(char *str)
{
    size_t  len = strlen(str);
    if(len > 0 && str[len -1]=='\n')    // we writng this function becaude when enterd the exit on cleint server it takes newline 'exit\n' ,so reomve this line we wrote this function 
    
        str[len - 1]='\0';
    if(len > 1 && str[len -2]=='r')
        str[len -2]='\0';
}
int main() {

    time_t now;
    struct tm *local;
    char buffer [4096];
    char response [1024];
    char time_string[100];
    int bytes ;
    time(&now); // get the current time using time function in it passing the address of varaible declard 

    local=localtime(&now); // convert to the local time
    strftime(time_string,sizeof(time_string),"%Y-%m-%d %H:%M:%S",local); //conver that structured time into readble string format 


    int server_fd, client_fd;
    struct sockaddr_in address;

    snprintf(response,sizeof(response),"HTTP/1.1 200 OK\r\n"  // uesd this function to print the HTMl content on browser 
             "Content-Type: text/html\r\n\r\n"
             "<html><body>"
             "<h1>Hello from Server!</h1>"
             "<p>Current server time: %s</p>"
             "</body></html>",time_string);
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    while (1)
    {
    client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
    



        memset(buffer,0,sizeof(buffer)); // read the http request and first empty the buffer 
        read(client_fd,buffer,sizeof(buffer)-1); // read data from client 
        printf("Raw HTTP Request Recived :\n...................\n");
        printf("%s\n",buffer);
        
         char method[10], path[1024], protocol[10];
        sscanf(buffer, "%s %s %s", method, path, protocol);
        newline(buffer);

        // Handle routes
        if (strcmp(path, "/hello") == 0) {
            snprintf(response, sizeof(response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n\r\n"
                     "<html><body><h1>Hello Page</h1></body></html>");
        }
        else if  (strcmp(path, "/bye") == 0){
            snprintf(response, sizeof(response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n\r\n"
                     "<html><body><h1>Goodbye Page</h1></body></html>");
        }

        else {
            // Default page with current time
            time_t now = time(NULL);
            struct tm *local = localtime(&now);
            char time_string[100];
            strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", local);

            snprintf(response, sizeof(response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n\r\n"
                     "<html><body>"
                     "<h1>Default Page</h1>"
                     "<p>Current server time: %s</p>"
                     "</body></html>", time_string);
        }
    

        write(client_fd, response, strlen(response));
        close(client_fd);
    }

    close(server_fd);
    return 0;
}