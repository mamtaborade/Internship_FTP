
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


void newline(char *str)
{
    size_t  len = strlen(str);
    if(len > 0 && str[len -1]=='\n')    // we writng this function becaude when enterd the exit on cleint server it takes newline 'exit\n' ,so reomve this line we wrote this function 
    
        str[len - 1]='\0';
    if(len > 1 && str[len -2]=='r')
        str[len -2]='\0';
}

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
    const char * welcome_msg="Welcome to the server!\n"; // welcome send msg 
    write(client_fd, welcome_msg,strlen(welcome_msg));

    char buffer[1024];
    int bytes;
    int is_exit=1;

    while ((bytes = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) { 
        buffer[bytes] ='\0';
        newline(buffer);
        printf("Client say : %s\n",buffer);
        
        if(strcmp(buffer,"exit")==0)
        {
            const char *shudtdown_msg ="Server is Shutting Down.....\n";
            send(client_fd,shudtdown_msg,strlen(shudtdown_msg),0);
            break ; // exit the while loop to terminate server 
        }
            
        send(client_fd, buffer, bytes, 0);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}