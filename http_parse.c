#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define PORT 8080

// Read until end of HTTP headers (\r\n\r\n) or buffer full
ssize_t recv_all_headers(int fd, char *buf, size_t cap) {
    size_t used = 0;
    int recv_count = 0; // Counter for recv() calls

    while (used + 1 < cap) {
        ssize_t n = recv(fd, buf + used, cap - used - 1, 0);
        if (n <= 0) {
            return n;  // error or client closed
        }
        used += (size_t)n;
        buf[used] = '\0';
        recv_count++; // Increment the recv() call counter

        if (strstr(buf, "\r\n\r\n")) {
            break; // end of headers
        }
    }

    printf("Number of recv() calls: %d\n", recv_count); // Print the number of recv() calls
    return (ssize_t)used;
}

int main(void) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(s);
        exit(1);
    }

    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *)&a, sizeof(a)) < 0) {
        perror("bind");
        close(s);
        exit(1);
    }

    if (listen(s, 16) < 0) {
        perror("listen");
        close(s);
        exit(1);
    }

    printf("Request parsing demo running on port %d\n", PORT);

    for (;;) {
        int c = accept(s, NULL, NULL);
        if (c < 0) {
            perror("accept");
            continue;
        }

        char req[8192];
        ssize_t n = recv_all_headers(c, req, sizeof(req));
        if (n <= 0) {
            close(c);
            continue;
        }

        // Print the raw HTTP request headers
        printf("Raw HTTP request headers:\n%s\n", req);

        char method[8], path[512], version[16];
        method[0] = path[0] = version[0] = '\0';

        sscanf(req, "%7s %511s %15s", method, path, version);
        printf("Method=%s Path=%s Version=%s\n", method, path, version);

        const char *body = "Parsed!\n";
        char hdr[200];
        snprintf(hdr, sizeof(hdr),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %zu\r\n"
                 "Connection: close\r\n\r\n",
                 strlen(body));

        send(c, hdr, strlen(hdr), 0);
        send(c, body, strlen(body), 0);
        close(c);
    }
}

