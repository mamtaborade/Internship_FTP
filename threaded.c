#include <stdio.h>   // printf ,perror,snprintf
#include <stdlib.h> //malloc,free,exit
#include <string.h> //strlen, strstr
#include <unistd.h> // close ,read,write
#include <pthread.h> // pthread _t,pthread_create,pthread_detach
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

ssize_t recv_all_headers(int fd, char *buf, size_t cap) {
    size_t used = 0;
    while (used + 1 < cap) {                                      // this function read fromthe socket unil it find the HTTP request headers end marke \r\n\r\n or until buffer fills 
                                                                // recv() reads raw data from the socket 
        ssize_t n = recv(fd, buf + used, cap - used - 1, 0);   // the request is stored in buf .
        if (n <= 0) return n;
        used += (size_t)n;
        buf[used] = '\0';
        if (strstr(buf, "\r\n\r\n")) break;
    }
    return (ssize_t)used;
}

void* worker(void* arg){
    int c = *(int*)arg; 
    free(arg);

    char req[4096];
    if (recv_all_headers(c, req, sizeof(req)) <= 0) {
        close(c);
        return NULL;
    }

    const char *body = "Hello from a thread!\n";
    char hdr[256];
    snprintf(hdr,sizeof(hdr),
      "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n"
      "Content-Length: %zu\r\nConnection: close\r\n\r\n",
      strlen(body));

    send(c, hdr, strlen(hdr), 0);
    send(c, body, strlen(body), 0);
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
        pthread_create(&t, NULL, worker, c);  // each incoming client is served  by seprate worker thread .
        
        pthread_detach(t);
    }
}
