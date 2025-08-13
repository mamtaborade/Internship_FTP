// Day 4: Fork-based HTTP Server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void handle_client(int client_fd) {
    char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                      "<html><body><h1>Hello from Forked Server!</h1></body></html>";
    write(client_fd, response, strlen(response));
    close(client_fd);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int ret;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 10);
    
    printf("program started..\n");
	ret = fork();
	if(ret == 0) {
		printf("child: fork() returned: %d\n", ret);
		printf("child: pid=%d\n", getpid());

	}
	else {
		printf("parent: fork() returned: %d\n", ret);
		//printf("parent: pid=%d\n", getpid());
		//printf("parent: parent pid=%d\n", getppid());
	}

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (fork() == 0) {
            handle_client(client_fd);
            exit(0);
        }
        close(client_fd);
    }

    close(server_fd);
    getchar();
    return 0;
}
