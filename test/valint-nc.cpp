/// @brief 参考别人的代码实现简单的nc
/// @origin_author valiantchend
/// @link https://github.com/valiantcheng/valiant-nc
/// @link https://www.bilibili.com/video/BV18g1sYGEZG 
#ifndef _VALINT_NC_
#define _VALINT_NC_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void error(const char *msg);
void run_client(const char *address, int port);
void run_udp_client(const char *address, int port);
void run_server(int port);
void run_udp_server(int port);

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s -l <port> udp|tcp (for server) or %s <address> <port> udp|tcp (for client)\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    int is_udp = strcmp(argv[3], "udp") == 0;

    if (strncmp(argv[1], "-l", 2) == 0) {
        int port = atoi(argv[2]);
        if (is_udp) {
            run_udp_server(port);
        } else {
            run_server(port);
        }
    } else {
        const char *address = argv[1];
        int port = atoi(argv[2]);
        if (is_udp) {
            run_udp_client(address, port);
        } else {
            run_client(address, port);
        }
    }

    return 0;
}

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void run_client(const char *address, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE]; // I/O memory buffer

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Socker creation failed");
    }

    // set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        error("Invalid address / Address not supported");
    }

    // connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Connection failed");
    }
    printf("Connected to server %s:%d\n", address, port);
    printf("Type 'exit' to close the connection.\n");

    // communication loop
    while (1) {
        printf("Client's message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // remove newline character

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Closing connection.\n");
            break;
        }

        // send message to server
        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            error("Send failed");
        }

        // receive response from server
        // Q: Why is the buffer size reduced by 1?
        ssize_t n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (n < 0) {
            error("Receive failed");
        } else if (n == 0) {
            printf("Server closed the connection.\n");
            break;
        }
        buffer[n] = '\0'; // null-terminate the received string
        printf("Server's response: %s\n", buffer);
    }

    close(sockfd);
    printf("Client disconnected.\n");
}

void run_server(int port) {
    int sockfd, newsockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE]; // I/O memory buffer

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Socket creation failed");
    }

    // set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // bind to any address
    server_addr.sin_port = htons(port);

    // bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Bind failed");
    }

    // listen for incoming connections
    if (listen(sockfd, 5) < 0) {
        error("Listen failed");
    }
    printf("Server listening on port %d\n", port);

    // accept connections in a loop
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
        if (newsockfd < 0) {
            error("Accept failed");
        }
        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        // printf("Type 'exit' to close the connection.\n");

        // communication loop with the client
        while (1) {
            ssize_t n = recv(newsockfd, buffer, BUFFER_SIZE - 1, 0);
            if (n < 0) {
                error("Receive failed");
            } else if (n == 0) {
                printf("Client disconnected.\n");
                break;
            }
            buffer[n] = '\0'; // null-terminate the received string
            printf("Client's message: %s\n", buffer);

            // echo back the message to the client
            if (send(newsockfd, buffer, n, 0) < 0) {
                error("Send failed");
            }
        }
        close(newsockfd);
        printf("Connection closed.\n");

        printf("Type 'exit' to close the Server or just press Enter to continue to listen.\n");
        printf("Server: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Server shutting down.\n");
            break;
        }
    }

    close(newsockfd);
    close(sockfd);
}

void run_udp_client(const char *address, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    // socklen_t addr_len;

    // create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("Socket creation failed");
    }

    // set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
        error("Invalid address / Address not supported");
    }
    // addr_len = sizeof(server_addr);

    printf("Connected to UDP server %s:%d\n", address, port);
    printf("Type 'exit' to close the connection.\n");

    // communication loop
    while (1) {
        printf("Client's message: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // remove newline character

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Closing connection.\n");
            break;
        }

        // send message to server
        if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            error("Send failed");
        }

        // receive response from server
        socklen_t addr_len = sizeof(server_addr);
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&server_addr, &addr_len);
        if (n < 0) {
            error("Receive failed");
        } else if (n == 0) {
            printf("Server closed the connection.\n");
            break;
        }
        buffer[n] = '\0'; // null-terminate the received string
        printf("Server's response: %s\n", buffer);
    }

    close(sockfd);
    printf("Client disconnected.\n");
}       

void run_udp_server(int port) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("Socket creation failed");
    }

    // set server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // bind to any address
    server_addr.sin_port = htons(port);

    // bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Bind failed");
    }

    printf("UDP Server listening on port %d\n", port);

    // communication loop
    while (1) {
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            error("Receive failed");
        } else if (n == 0) {
            printf("Client disconnected.\n");
            break;
        }
        buffer[n] = '\0'; // null-terminate the received string
        printf("Client's message: %s\n", buffer);

        printf("You: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        // 检查是否输入了 "exit"
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Closing connection...\n");
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&client_addr, client_len);
            break;
        }
        // echo back the message to the client
        if (sendto(sockfd, buffer, n, 0, (struct sockaddr *)&client_addr, client_len) < 0) {
            error("Send failed");
        }
        printf("Sent response to %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    close(sockfd);
}

#endif // _VALINT_NC_