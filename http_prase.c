
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>

#define PORT 8080

// Read until end of HTTP headers (\r\n\r\n) or buffer full
ssize_t recv_all_headers(int fd, char *buf, size_t cap) {
    size_t used = 0;
    int  recv_count  = 0;  // counter for recv calls 
    while (used + 1 < cap) {
        ssize_t n = recv(fd, buf + used, cap - used - 1, 0);
        if (n <= 0) {
            return n;  // error or client closed    // if nothing is recvied or error happend , stop and return 
        }
         recv_count ++ ;  // increase counter every time recv() suceeds
        used += (size_t)n;              // increase used count 
        buf[used] = '\0';               // always null terminate(\0) the string sowe can safely use string fucntion like str str 

        if (strstr(buf, "\r\n\r\n")) {
            break; // end of headers
              //  cheking the http headers always rend with "\r\n\r\n"
        }
    }
    printf("recv() was called %d time to get headers \n",recv_count);
   // fflush(stdout);  
    printf("==== RAW HTTP Request Headers ===\n%s\n",buf);   // this help us to see what the browser send when we type a url 
    return (ssize_t)used;    // this return how many  bytes of headers we actually read 
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
         // char buffer 
        ssize_t n = recv_all_headers(c, req, sizeof(req));
        if (n <= 0) {
            close(c);
            continue;
        }

        char method[8], path[512], version[16] ;
           method[0] = path[0] = version[0] = '\0';

        sscanf(req, "%7s %511s %15s", method, path, version);
        printf("Http methods = %s\n",method);  
           // task 2 only  HTTP methods printing ..

        //---------------Parse Host -------------------//


        // assume `req` contains full HTTP request
char *host_start = strstr(req, "Host: ");  // we extracting host value from http rwa request line  thsi is logic for extracting host value .
if (host_start) {
    host_start += 5;  // move pointer after "Host:"
    while (*host_start == ' ') host_start++;  // skip spaces

    char host_value[256];
    int i = 0;

    // copy until newline or string end
    while (*host_start && *host_start != '\r' && *host_start != '\n' && i < 255) {
        host_value[i++] = *host_start++;
    }
    host_value[i] = '\0';

    printf("Host = %s\n", host_value);
}

char * query = NULL ;
char * question_mark =strchr(path,'?');
if(question_mark !=NULL)
{
    *question_mark='\0'; // termonat path at '?'
    }
printf("path=%s\n",path);
if(query)
{
    printf("Query=%s\n",query);
}


    // -----------Handle Methods--------------
      char hdr[512];
      char body [1024];
      
     
        if (strcmp(method,"GET")!=0){
        strcpy(body,"405 Method not allowed \n");    // if rahter than GET method fpund it will show error
        snprintf(hdr, sizeof(hdr),
                 "HTTP/1.1 405 MethodNot allowed \r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %zu\r\n"
                 "Connection: close\r\n\r\n",
                 strlen(body));
        }
        else if (strcmp(path, "/hello") == 0) {
            strcpy(body,"<html><body><h1>Hello Page</h1></body></html>");
            snprintf(hdr, sizeof(hdr),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n\r\n"
                       "Content-Length: %zu\r\n"
                 "Connection: close\r\n\r\n",strlen(body));
        }
        else if (strcmp(path,"/time")==0)
        {
            time_t now = time(NULL);
            char *tstr =ctime(&now);
            tstr[strcspn(tstr,"\n")]='\0';  // remove this line 

            snprintf(body,sizeof(body),"%s",tstr);
            snprintf(hdr,sizeof(hdr),
            "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %zu\r\n"
                 "Connection: close\r\n\r\n",
                 strlen(body));
        }
        else{
            strcpy(body,"Welcome to my Server! \n");
            snprintf(hdr,sizeof(hdr),
              "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n",
             strlen(body));
        }
        send(c, hdr, strlen(hdr), 0);
        send(c, body, strlen(body), 0);
        close(c);
        continue ; // skip to next request 
    }
}

