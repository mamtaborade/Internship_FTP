// day2_demo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    char cmd[] = "PORT 127,0,0,1,195,80";
    int h1,h2,h3,h4,p1,p2;
    sscanf(cmd, "PORT %d,%d,%d,%d,%d,%d", &h1,&h2,&h3,&h4,&p1,&p2);
    char ip[32];
    sprintf(ip, "%d.%d.%d.%d", h1,h2,h3,h4);
    int port = p1*256 + p2;

    printf("Client requested data connection:\n");
    printf("IP   = %s\n", ip);
    printf("Port = %d\n", port);


    //  send control reply 

    printf("200 PORT command sucessful\r\n");

    


    // printf("Client requested PORT IP=%s, port=%d\n", ip, port);

    // Example: connect to that IP/port
    int sock = socket(AF_INET, SOCK_STREAM, 0); 
    if(sock < 0)
    {
        perror("Socket");
        return 1 ;
    }
     // Establih TCP connection 
    struct sockaddr_in data_addr;
    memset(&data_addr,0,sizeof(data_addr));
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip, &data_addr.sin_addr)<=0)
    {
        perror("inet_pton");
        close(sock);
        return 1 ;
    }

    
    if(connect(sock,(struct sockaddr*)&data_addr, sizeof(data_addr))==0)
    {
        printf("Data connection established ! \n");

        // Example : send test message (like directory listing )
        const char *msg ="Hello from FTP data connection!\r\n";
        write(sock,msg,strlen(msg));
        
    }
    else {
        perror("connect");
    }

    close(sock);
    return 0;
}