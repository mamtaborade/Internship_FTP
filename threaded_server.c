#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
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

void* worker(void* arg){
    int c = *(int*)arg; 
    free(arg);

    char req[8192];
    ssize_t hn = recv_all_headers(c, req, sizeof(req));
    if (hn <= 0) {
        close(c);
        return NULL;
    }

    // Parse request line and headers
    char method[8], path[256];
    if (sscanf(req, "%7s %255s", method, path) < 2) {
        close(c);
        return NULL;
    }

    // Only handle POST method for body reading, else respond with a simple message
    if (strcasecmp(method, "POST") == 0) {
        // Find Content-Length header
        char *cl = strcasestr(req, "Content-Length:");
        long len = 0;
        if (cl) {
            len = strtol(cl + 15, NULL, 10);
        }
        if (len < 0 || len > MAX_BODY_SIZE) {
            // Reject too large or invalid bodies
            close(c);
            return NULL;
        }

        // Find start of body in already received data
        char *body_start = strstr(req, "\r\n\r\n");
        size_t already = 0;
        if (body_start) {
            already = hn - (body_start - req) - 4;
        }

        // Allocate buffer for body
        char *body = malloc((size_t)len + 1);
        if (!body) {
            close(c);
            return NULL;
        }

        // Copy already received part of body
        size_t copied = 0;
        if (already > 0) {
            if (already > (size_t)len) already = (size_t)len;
            memcpy(body, body_start + 4, already);
            copied = already;
        }

        // Receive remaining body bytes
        if (copied < (size_t)len) {
            if (recv_exact(c, body + copied, (size_t)len - copied) < 0) {
                free(body);
                close(c);
                return NULL;
            }
        }
        body[len] = '\0';

        // Task 3: Save body to data.txt
        FILE *f = fopen("data.txt", "wb");
        if (f) {
            fwrite(body, 1, (size_t)len, f);
            fclose(f);
        } else {
            perror("fopen");
        }

        // Prepare response
        char msg[9000];
        snprintf(msg, sizeof(msg), "Received POST data (%ld bytes):\n%s\n", len, body);

        char hdr[256];
        snprintf(hdr, sizeof(hdr),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n",
            strlen(msg));

        //send(c, hdr, strlen(hdr), 0);
        //send(c, msg, strlen(msg), 0);
        if (send(c, hdr, strlen(hdr), 0) < 0) perror("send header");
	if (send(c, msg, strlen(msg), 0) < 0) perror("send body");

        free(body);
    } else {
        // For non-POST requests, simple hello message
        const char *body = "Hello from a thread!\n";
        char hdr[256];
        snprintf(hdr,sizeof(hdr),
          "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n"
          "Content-Length: %zu\r\nConnection: close\r\n\r\n",
          strlen(body));

        send(c, hdr, strlen(hdr), 0);
        send(c, body, strlen(body), 0);
    }

    close(c);
    return NULL;
}

int main(){
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

    for(;;){
        int *c = malloc(sizeof(int));
        if (!c) continue;
        *c = accept(s, NULL, NULL);
        if (*c < 0) { perror("accept"); free(c); continue; }

        pthread_t t;
        pthread_create(&t, NULL, worker, c);
        pthread_detach(t);
    }
}
