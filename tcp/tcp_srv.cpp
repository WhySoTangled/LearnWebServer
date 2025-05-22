/***********
 * a simple example of TCP server
 *************/
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

const int PORT = 8080;         // 监听端口
const int BUFFER_SIZE = 1024;  // 缓冲区大小

int main() {
    // 创建套接字
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    // 设置套接字选项（允许地址重用）
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定地址和端口
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // 监听所有网络接口
    address.sin_port = htons(PORT);        // 端口字节序转换
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    // 开始监听（最大连接队列为5）
    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }
    std::cout << "Server listening on port " << PORT << std::endl;

    // 接受客户端连接
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        std::cerr << "Accept failed" << std::endl;
        return 1;
    }
    std::cout << "New client connected" << std::endl;

    // 接收和发送数据
    char buffer[BUFFER_SIZE] = {0};
    while(true) {
        // 接收数据
        ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }
        std::cout << "Received: " << buffer << std::endl;

        // 发送响应（原样返回）
        send(client_fd, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);  // 清空缓冲区
    }

    // 关闭套接字
    close(client_fd);
    close(server_fd);
    return 0;
}