#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>

#define PORT 8080
#define BUF_SIZE 8192


ssize_t recv_all_headers(int fd, char *buf, size_t cap) {
    size_t used = 0;
    int count = 0;
    while (used + 1 < cap) {
        ssize_t n = recv(fd, buf + used, cap - used - 1, 0);
        if (n <= 0) return n;
        used += (size_t)n;
        buf[used] = '\0';
        count++;
        if (strstr(buf, "\r\n\r\n")) break;
    }
    printf("recv() called %d times\n", count);
    return (ssize_t)used;
}


void handle_client(int cfd, struct sockaddr_in client_addr) {
    char req[BUF_SIZE];

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(client_addr.sin_port);

    printf("\n--- New Client Connected ---\n");
    printf("Child PID: %d\n", getpid());
    printf("Client IP: %s\n", client_ip);
    printf("Client Port: %d\n", client_port);
    printf("-----------------------------\n");

    ssize_t n = recv_all_headers(cfd, req, sizeof(req));
    if (n <= 0) {
        close(cfd);
        exit(0);
    }

    printf("\n--- Full HTTP Request ---\n%s\n-------------------------\n", req);

    char method[16], path[512], version[16];
    sscanf(req, "%15s %511s %15s", method, path, version);
    printf("Method=%s Path=%s Version=%s\n", method, path, version);

    
    if (strcmp(method, "GET") != 0) {
        const char *resp_body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        char hdr[256];
        snprintf(hdr, sizeof(hdr),
                 "HTTP/1.1 405 Method Not Allowed\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: %zu\r\n"
                 "Connection: close\r\n\r\n",
                 strlen(resp_body));
        send(cfd, hdr, strlen(hdr), 0);
        send(cfd, resp_body, strlen(resp_body), 0);
        close(cfd);
        exit(0);
    }

    
    char query[512];
    char *q = strchr(path, '?');
    if (q) {
        strncpy(query, q + 1, sizeof(query)-1);
        *q = '\0';
        printf("Query=%s\n", query);
    } else query[0] = '\0';

    
    char body[1024];
    time_t t = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&t));

    if (strcmp(path, "/hello") == 0) {
        snprintf(body, sizeof(body),
                 "<html><body><h1>Hello Student!</h1>"
                 "<p>Client IP: %s</p><p>Query: %s</p>"
                 "<p>Time: %s</p></body></html>",
                 client_ip, query, time_str);
    } else if (strcmp(path, "/time") == 0) {
        snprintf(body, sizeof(body),
                 "<html><body><h1>Current Server Time</h1>"
                 "<p>%s</p></body></html>", time_str);
    } else {
        snprintf(body, sizeof(body),
                 "<html><body><h1>Unknown Path!</h1>"
                 "<p>Requested Path: %s</p><p>Time: %s</p></body></html>",
                 path, time_str);
    }

    char hdr[256];
    snprintf(hdr, sizeof(hdr),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n",
             strlen(body));

    send(cfd, hdr, strlen(hdr), 0);
    send(cfd, body, strlen(body), 0);

    close(cfd);
    printf("Child PID %d closed connection.\n", getpid());
    exit(0);
}

int main(void) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) { perror("bind"); exit(1); }
    if (listen(s, 16) < 0) { perror("listen"); exit(1); }

    signal(SIGCHLD, SIG_IGN); 

    printf("Multi-client HTTP server running on http://localhost:%d\n", PORT);

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int c = accept(s, (struct sockaddr*)&client_addr, &len);
        if (c < 0) { perror("accept"); continue; }

        pid_t pid = fork();
        if (pid == 0) {
            close(s); 
            handle_client(c, client_addr);
        } else if (pid > 0) {
            close(c); 
        } else {
            perror("fork"); close(c);
        }
    }

    close(s);
    return 0;
}