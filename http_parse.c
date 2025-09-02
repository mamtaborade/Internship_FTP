#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <pthread.h>

#define PORT 8080

// Task 1: Modify recv_all_headers to count recv() calls and print raw headers
ssize_t recv_all_headers(int fd, char *buf, size_t cap) {
    size_t used = 0;
    int recv_calls = 0;
    while (used + 1 < cap) {
        ssize_t n = recv(fd, buf + used, cap - used - 1, 0);
        recv_calls++;
        if (n <= 0) {
            if (recv_calls > 0)
                printf("[recv_all_headers] recv calls: %d\n", recv_calls);
            return n;  // error or client closed
        }
        used += (size_t)n;
        buf[used] = '\0';
        if (strstr(buf, "\r\n\r\n")) {
            printf("[recv_all_headers] recv calls: %d\n", recv_calls);
	    printf("Raw HTTP request headers:\n%s\n", buf);
            break; // end of headers
        }
    }
    return (ssize_t)used;
}

// Helper: Trim leading/trailing whitespace
void trim(char *str) {
    // Trim leading
    while(isspace((unsigned char)*str)) str++;
    // Trim trailing
    char *end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) *end-- = '\0';
}

// Task 7: Handle client connection in a thread
void *handle_client(void *arg) {
    int c = *(int *)arg;
    free(arg);
    
    char req[8192];
    ssize_t n = recv_all_headers(c, req, sizeof(req));
    if (n <= 0) {
        close(c);
        return NULL;
    }
    
    // Task 2: Extract Method, Path, Version
    char method[8], path[512], version[16];
    method[0] = path[0] = version[0] = '\0';
    
    sscanf(req, "%7s %511s %15s", method, path, version);
    
    // Print only HTTP method
    printf("Method=%s\n", method);
    
    // Reject unsupported methods (only GET and POST supported)
    if (strcmp(method, "GET") != 0 && strcmp(method, "POST") != 0) {
        const char *body = "Method Not Allowed\n";
        char hdr[200];
        snprintf(hdr, sizeof(hdr),
                 "HTTP/1.1 405 Method Not Allowed\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %zu\r\n"
                 "Connection: close\r\n\r\n",
                 strlen(body));
        send(c, hdr, strlen(hdr), 0);
        send(c, body, strlen(body), 0);
        close(c);
        return NULL;
    }
    
    // Task 3: Parse Host header
    char *host_line = strstr(req, "\nHost:");
    if (!host_line) host_line = strstr(req, "\nhost:"); // case-insensitive fallback
    if (host_line) {
        host_line += 6; // skip "Host:"
        while (*host_line == ' ' || *host_line == '\t') host_line++;
        char host[512];
        int i = 0;
        while (host_line[i] != '\r' && host_line[i] != '\n' && i < (int)sizeof(host) - 1) {
            host[i] = host_line[i];
            i++;
        }
        host[i] = '\0';
        trim(host);
        printf("Host=%s\n", host);
    } else {
        printf("Host header not found\n");
    }
    
    // Task 5: Handle query parameters (for GET)
    char *query = NULL;
    char path_only[512];
    strncpy(path_only, path, sizeof(path_only));
    path_only[sizeof(path_only)-1] = '\0';
    query = strchr(path_only, '?');
    if (query) {
        *query = '\0'; // terminate path_only at '?'
        query++;       // query points to query string
        printf("Query=%s\n", query);
    }
    
    // Task 6: Handle POST request body
    char *body_data = NULL;
    size_t content_length = 0;
    if (strcmp(method, "POST") == 0) {
        // Find Content-Length header
        char *cl = strcasestr(req, "Content-Length:");
        if (cl) {
            cl += strlen("Content-Length:");
            while (*cl == ' ' || *cl == '\t') cl++;
            content_length = (size_t)atoi(cl);
            printf("Content-Length=%zu\n", content_length);
        } else {
            content_length = 0;
        }
        
        // Find end of headers
        char *end_headers = strstr(req, "\r\n\r\n");
        if (end_headers) {
            end_headers += 4; // skip \r\n\r\n
            size_t already_read = n - (end_headers - req);
            if (content_length > 0) {
                body_data = malloc(content_length + 1);
                if (!body_data) {
                    perror("malloc");
                    close(c);
                    return NULL;
                }
                // Copy already read body part
                if (already_read > 0) {
                    size_t to_copy = (already_read > content_length) ? content_length : already_read;
                    memcpy(body_data, end_headers, to_copy);
                }
                // Read remaining body bytes
                size_t total_read = already_read;
                while (total_read < content_length) {
                    ssize_t r = recv(c, body_data + total_read, content_length - total_read, 0);
                    if (r <= 0) break;
                    total_read += (size_t)r;
                }
                 body_data[content_length] = '\0';
                printf("POST body: %s\n", body_data);
            }
        }
    }
    
    // Task 4: Serve different responses based on path
    const char *response_body = NULL;
    char time_str[128];
    if (strcmp(path_only, "/hello") == 0) {
        response_body = "Hello Student!\n";
    } else if (strcmp(path_only, "/time") == 0) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S\n", tm_info);
        response_body = time_str;
    } else {
        response_body = "Unknown Path!\n";
    }
    
    char hdr[256];
    snprintf(hdr, sizeof(hdr),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n",
             strlen(response_body));
             
    send(c, hdr, strlen(hdr), 0);
    send(c, response_body, strlen(response_body), 0);
    
    if (body_data) free(body_data);
    close(c);
    return NULL;
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
    
    // Task 7: Multi-client handling with pthreads
    for (;;) {
        int *c = malloc(sizeof(int));
        if (!c) {
            perror("malloc");
            continue;
        }
        *c = accept(s, NULL, NULL);
        if (*c < 0) {
            perror("accept");
            free(c);
            continue;
        }
        
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, c) != 0) {
            perror("pthread_create");
            close(*c);
            free(c);
            continue;
        }
        pthread_detach(tid); // resources freed when thread exits
    }
    
    close(s);
    return 0;
}


