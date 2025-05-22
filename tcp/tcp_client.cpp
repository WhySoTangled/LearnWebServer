/***********
 * a simple example of TCP client
 *************/
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char* SERVER_IP = "127.0.0.1";  // 服务器IP
const int PORT = 8080;                // 服务器端口
const int BUFFER_SIZE = 1024;         // 缓冲区大小

int main() {
    // 创建套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    // 连接服务器
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }
    std::cout << "Connected to server" << std::endl;

    // 发送和接收数据
    char buffer[BUFFER_SIZE] = {0};
    const char* message = "Hello from client!";
    
    // 发送数据
    send(sock, message, strlen(message), 0);
    std::cout << "Message sent: " << message << std::endl;

    // 接收响应
    ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE);
    std::cout << "Server response: " << buffer << std::endl;

    // 关闭套接字
    close(sock);
    return 0;
}