#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>


#define PORT 8080 

void handle_client(int client_fd)

{
    char buffer [1024];
    char response []= "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                      "<html><body><h1>Hello from Forked Server!</h1></body></html>";

                      write(client_fd,response,strlen(response));
                      close(client_fd);


           
            close(client_fd);
}

int main ( ){
    
    int server_fd, client_fd ;
    char buffer [1024];

    struct sockaddr_in address ,client_addr;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET,SOCK_STREAM,0);
    address.sin_family =AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd,(struct sockaddr*)&address,sizeof(address));
    listen(server_fd,10);

    while(1)

    {
            client_fd =accept(server_fd,(struct sockaddr*)&address,&addrlen);
        
            if(fork()==0){

                 while(1)
            {
                memset(buffer,0,sizeof(buffer));
                int bytes_read=read(client_fd,buffer,sizeof(buffer)-1); // read data from client 

                if(bytes_read <=0)
                {
                    break ; // client close connection 
                }

                buffer[bytes_read]='\0';
                printf("Client said : %s\n",buffer);

                    // if client say exit stop serving 
                if(strncmp(buffer,"exit",4)==0)
                {
                    printf("Client requested to close connection.\n");
                    break ;
                }
                write(client_fd,buffer,strlen(buffer));

            }

                // child process 
            pid_t pid =getpid();

            // get client IP and port 
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET,&(client_addr.sin_addr),client_ip,INET_ADDRSTRLEN);
            int client_port=ntohs(client_addr.sin_port);
            
            // get current time 
            time_t now = time(NULL);
            char *time_str = ctime(&now);
            time_str[strlen(time_str)-1]='\0'; // remov e newline 
            
            printf("\n[Child %d] New Connection :\n",pid);
            printf("Client IP : %s\n",client_ip);
            printf("Client port : %d\n",client_port);
            printf("Start Time : %s\n",time_str);

            handle_client(client_fd);
                exit(0);
            
            }
            close(client_fd);
        }
close(server_fd);
return 0 ;

}