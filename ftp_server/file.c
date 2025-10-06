#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define CTRL_PORT 2121
#define DATA_PORT 2020
#define BUFFER 1024



void send_list_data(int data_fd) {
    FILE *fp = popen("ls -l", "r");
    if (!fp) {
        const char *msg = "450 LIST command failed.\r\n";
        send(data_fd, msg, strlen(msg), 0);
        return;
    }

    char line[BUFFER];
    while (fgets(line, sizeof(line), fp)) {
        send(data_fd, line, strlen(line), 0);
    }
    pclose(fp);
}

void send_file_data(int data_fd, const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        const char *msg = "550 File not found.\r\n";
        send(data_fd, msg, strlen(msg), 0);
        return;
    }

    char buf[BUFFER];
    while (fgets(buf, sizeof(buf), fp)) {
        send(data_fd, buf, strlen(buf), 0);
    }
    fclose(fp);
}



int setup_data_socket() {
    int data_fd;
    struct sockaddr_in data_addr = {0};

    if ((data_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Data socket creation failed");
        return -1;
    }

    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = INADDR_ANY;
    data_addr.sin_port = htons(DATA_PORT);

    if (bind(data_fd, (struct sockaddr*)&data_addr, sizeof(data_addr)) < 0) {
        perror("Data bind failed");
        close(data_fd);
        return -1;
    }

    if (listen(data_fd, 1) < 0) {
        perror("Data listen failed");
        close(data_fd);
        return -1;
    }

    return data_fd;
}



int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[BUFFER];

    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(CTRL_PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("FTP server running on port..... %d...\n", CTRL_PORT);

    
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
    if (client_fd < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    send(client_fd, "220 Simple FTP Server Ready\r\n", 29, 0);


    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;

        printf("Client: %s", buffer);

        if (strncmp(buffer, "USER", 4) == 0) {
            send(client_fd, "331 Username OK, need password\r\n", 32, 0);
        }
        else if (strncmp(buffer, "PASS", 4) == 0) {
            send(client_fd, "230 Login successful\r\n", 23, 0);
        }
        else if (strncmp(buffer, "QUIT", 4) == 0) {
            send(client_fd, "221 Goodbye\r\n", 13, 0);
            break;
        }
        else if (strncmp(buffer, "LIST", 4) == 0) {
            send(client_fd, "150 Opening data connection for LIST\r\n", 38, 0);

            int data_fd = setup_data_socket();
            if (data_fd < 0) {
                send(client_fd, "425 Can't open data connection\r\n", 32, 0);
                continue;
            }

            
            int client_data = accept(data_fd, NULL, NULL);
            if (client_data >= 0) {
                send_list_data(client_data);
                close(client_data);
            }
            close(data_fd);

            send(client_fd, "226 Directory send OK\r\n", 24, 0);
        }
        else if (strncmp(buffer, "RETR", 4) == 0) {
            char filename[256];
            sscanf(buffer + 5, "%255s", filename);

            send(client_fd, "150 Opening data connection for RETR\r\n", 38, 0);

            int data_fd = setup_data_socket();
            if (data_fd < 0) {
                send(client_fd, "425 Can't open data connection\r\n", 32, 0);
                continue;
            }

            int client_data = accept(data_fd, NULL, NULL);
            if (client_data >= 0) {
                send_file_data(client_data, filename);
                close(client_data);
            }
            close(data_fd);

            send(client_fd, "226 Transfer complete\r\n", 24, 0);
        }
        else {
            send(client_fd, "502 Command not implemented\r\n", 29, 0);
        }
    }

    close(client_fd);
    close(server_fd);
    printf("Server shut down.\n");
    return 0;
}
