#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_BODY_SIZE (1024*1024)  // 1 MB safety limit

// Read headers until \r\n\r\n
ssize_t recv_all_headers(int fd, char *buf, size_t cap) {
    size_t used = 0;
    while (used + 1 < cap) {
        ssize_t n = recv(fd, buf + used, cap - used - 1, 0);
        if (n <= 0) return n;
        used += (size_t)n;
        buf[used] = '\0';
        if (strstr(buf, "\r\n\r\n")) break;
    }
    return (ssize_t)used;
}

// Read exactly 'need' bytes
ssize_t recv_exact(int fd, char *buf, size_t need) {
    size_t got = 0;
    while (got < need) {
        ssize_t n = recv(fd, buf + got, need - got, 0);
        if (n <= 0) return -1;
        got += n;
    }
    return (ssize_t)got;
}

int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = INADDR_ANY;
    a.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr*)&a, sizeof(a)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(s, 16) < 0) {
        perror("listen"); exit(1);
    }

    printf("POST server running on port %d\n", PORT);

    for (;;) {
        int c = accept(s, NULL, NULL);
        if (c < 0) { perror("accept"); continue; }

        char head[8192];
        ssize_t hn = recv_all_headers(c, head, sizeof(head));
        if (hn <= 0) { close(c); continue; }

        char method[8], path[256];
        if (sscanf(head, "%7s %255s", method, path) < 2) {
            close(c); continue;
        }

        char *cl = strcasestr(head, "Content-Length:");
        long len = 0;
        if (cl) len = strtol(cl + 15, NULL, 10);
        if (len < 0 || len > MAX_BODY_SIZE) {
            close(c); continue;  // reject too large bodies
        }

        char *body_start = strstr(head, "\r\n\r\n");
        size_t already = 0;
        if (body_start) already = hn - (body_start - head) - 4;

        char *body = malloc((size_t)len + 1);
        if (!body) { close(c); continue; }

        size_t copied = 0;
        if (already > 0) {
            if (already > (size_t)len) already = (size_t)len;
            memcpy(body, body_start + 4, already);
            copied = already;
        }
        if (copied < (size_t)len) {
            if (recv_exact(c, body + copied, (size_t)len - copied) < 0) {
                free(body); close(c); continue;
            }
        }
        body[len] = '\0';

        // Prepare response
        char msg[9000];
        snprintf(msg, sizeof(msg), "You POSTed: %s\n", body);

        char hdr[256];
        snprintf(hdr, sizeof(hdr),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n",
            strlen(msg));

        send(c, hdr, strlen(hdr), 0);
        send(c, msg, strlen(msg), 0);

        free(body);
        close(c);
    }
}

