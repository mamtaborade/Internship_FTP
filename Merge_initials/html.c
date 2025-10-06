#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void handle_client(int client_fd, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];

   
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(client_addr.sin_port);

    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("\n--- New Client Connected ---\n");
    printf("Child PID: %d\n", getpid());
    printf("Client IP: %s\n", client_ip);
    printf("Client Port: %d\n", client_port);
    printf("Connection Time: %s\n", time_str);
    printf("-----------------------------\n");

    // Send welcome message as HTML
char body[512];
snprintf(body, sizeof(body),
         "<html><body>"
         "<h1>Welcome to Meenal's Server!</h1>"
         "<p>Type 'exit' to disconnect </p>"
         "</body></html>");

char response[1024];
snprintf(response, sizeof(response),
         "HTTP/1.1 200 OK\r\n"
         "Content-Type: text/html\r\n"
         "Connection: keep-alive\r\n\r\n"
         "%s", body);

write(client_fd, response, strlen(response));
fsync(client_fd);



    


    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break; 

        buffer[bytes] = '\0';
        printf("Client [%d]: %s", getpid(), buffer);

        
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client [%d] requested exit.\n", getpid());
            break;
        }

        
        if (strncmp(buffer, "GET ", 4) == 0) {
            printf("\n--- Full HTTP Request ---\n%s\n", buffer);

           
            char method[16], path[128];
            sscanf(buffer, "%s %s", method, path);

            now = time(NULL);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
            char body[1024];

            if (strcmp(path, "/hello") == 0) {
                snprintf(body, sizeof(body),
                         "<html><body><h1>Hello Page</h1>"
                         "<p>Client IP: %s</p><p>Time: %s</p></body></html>",
                         client_ip, time_str);
            } else if (strcmp(path, "/bye") == 0) {
                snprintf(body, sizeof(body),
                         "<html><body><h1>Goodbye Page</h1>"
                         "<p>Client IP: %s</p><p>Time: %s</p></body></html>",
                         client_ip, time_str);
            } else {
                snprintf(body, sizeof(body),
                         "<html><body><h1>Default Page</h1>"
                         "<p>Client IP: %s</p><p>Time: %s</p></body></html>",
                         client_ip, time_str);
            }

          
            char response[2048];
            int len = snprintf(response, sizeof(response),
                               "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: %zu\r\n"
                               "Connection: keep-alive\r\n\r\n"
                               "%s",
                               strlen(body), body);
            write(client_fd, response, len);
        } else {
            
            send(client_fd, buffer, bytes, 0);
        }
    }

    close(client_fd);
    printf("Child PID %d closed connection.\n", getpid());
    exit(0);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);

    signal(SIGCHLD, SIG_IGN); 

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("Socket creation failed"); exit(1); }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed"); close(server_fd); exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed"); close(server_fd); exit(1);
    }

    printf("Persistent Multi-client HTTP Server running on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_fd < 0) continue;

        pid_t pid = fork();
        if (pid == 0) {
            close(server_fd);
            handle_client(client_fd, client_addr);
        } else if (pid > 0) {
            close(client_fd);
        } else {
            perror("Fork failed");
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
