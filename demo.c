#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

// Read headers until \r\n\r\n or buffer full
ssize_t recv_all_headers(int fd, char *buf, size_t cap, size_t *header_len) {
    size_t used = 0;
    *header_len = 0;

    while (used + 1 < cap) {
        ssize_t n = recv(fd, buf + used, cap - used - 1, 0);
        if (n <= 0) return n;  // connection closed or error

        used += (size_t)n;
        buf[used] = '\0';  // keep buffer null-terminated

        char *end = strstr(buf, "\r\n\r\n");
        if (end) {
            *header_len = (end + 4) - buf;  // header length including CRLFCRLF
            return (ssize_t)used;           // total bytes read (headers + maybe body)
        }
    }
    return -1; // headers too large
}

void* worker(void* arg) {
    int c = *(int*)arg;
    free(arg);

    char req[8192];   // big enough for headers + body
    size_t header_len = 0;
    ssize_t total = recv_all_headers(c, req, sizeof(req), &header_len);
    if (total <= 0) {
        close(c);
        return NULL;
    }

    // ---- Parse Content-Length ----
    int content_length = 0;
    char *cl = strcasestr(req, "Content-Length:");
    if (cl) {
        sscanf(cl, "Content-Length: %d", &content_length);
    }

    // ---- Read remaining body if not fully received ----
    ssize_t already = total - header_len;
    ssize_t need = content_length - already;
    if (need > 0 && header_len + content_length < sizeof(req)) {
        ssize_t n = recv(c, req + total, need, 0);
        if (n > 0) total += n;
    }

    // ---- Extract body safely ----
    char *body = req + header_len;
    if (content_length > 0) {
        body[content_length] = '\0';  // null terminate body
        printf("Received body: %s\n", body);
    }

    // ---- Send Response ----
    const char *resp_body = (content_length > 0) ? body : "Hello from server!\n";
    char hdr[256];
    snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n",
        strlen(resp_body));

    send(c, hdr, strlen(hdr), 0);
    send(c, resp_body, strlen(resp_body), 0);

    close(c);
    return NULL;
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

    if (bind(s, (struct sockaddr*)&a, sizeof(a)) < 0) { perror("bind"); exit(1); }
    if (listen(s, 64) < 0) { perror("listen"); exit(1); }

    printf("pthread server on %d\n", PORT);

    for (;;) {
        int *c = malloc(sizeof(int));
        if (!c) continue;
        *c = accept(s, NULL, NULL);
        if (*c < 0) { perror("accept"); free(c); continue; }

        pthread_t t;
        pthread_create(&t, NULL, worker, c);
        pthread_detach(t);
    }
}
