#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 8080
#define BUF_SIZE 4096

const char* get_content_type(const char *filename) {
    if (strstr(filename, ".html")) return "text/html";
    if (strstr(filename, ".txt"))  return "text/plain";
    if (strstr(filename, ".jpg"))  return "image/jpeg";
    if (strstr(filename, ".jpeg")) return "image/jpeg";
    if (strstr(filename, ".png"))  return "image/png";
    return "application/octet-stream";
}

void send_file(int client_fd, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        char not_found[] =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html><body><h1>404 File Not Found</h1></body></html>";
        write(client_fd, not_found, strlen(not_found));
        return;
    }

    char header[256];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", get_content_type(filename));
    write(client_fd, header, strlen(header));

    char buf[BUF_SIZE];
    int n;
    while ((n = fread(buf, 1, BUF_SIZE, fp)) > 0) {
        write(client_fd, buf, n);
    }
    fclose(fp);
}

void handle_client(int client_fd) {
    char buffer[BUF_SIZE];

    int valread = read(client_fd, buffer, BUF_SIZE - 1);
    if (valread <= 0) {
        close(client_fd);
        exit(0);
    }
    buffer[valread] = '\0';
    printf("Request:\n%s\n", buffer);

    char method[16], path[128], protocol[16];
    sscanf(buffer, "%s %s %s", method, path, protocol);

    char filename[128];
    if (strcmp(path, "/") == 0) strcpy(filename, "index.html");
    else snprintf(filename, sizeof(filename), "%s", path + 1);

    send_file(client_fd, filename);
    close(client_fd);
    exit(0);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address, client_addr;
    socklen_t addrlen = sizeof(client_addr);

   
    signal(SIGCHLD, SIG_IGN);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("Socket failed"); exit(1); }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed"); close(server_fd); exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed"); close(server_fd); exit(1);
    }

    printf("Fork-based HTTP Server running on http://localhost:%d\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
        if (client_fd < 0) continue;

        pid_t pid = fork();
        if (pid == 0) {
            
            close(server_fd);
            handle_client(client_fd);
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
