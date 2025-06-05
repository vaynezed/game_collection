#include "base/ables.hpp"
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

template <>
std::wstring str(const bool& ele)
{
    std::wstring result;
    if (ele == true) {
        result = std::wstring(L"true");
    } else {
        result = std::wstring(L"false");
    }
    return result;
}

std::string format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    // 使用vsnprintf计算所需缓冲区大小
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    if (size < 0) {
        throw std::runtime_error("Format error");
    }

    // 创建足够大的缓冲区
    std::string result(size + 1, '\0');
    vsnprintf(&result[0], size + 1, fmt, args);
    va_end(args);

    return result;
}

class GameServer : public Server {
private:
    std::string host;
    int port;
    SOCKET socket_fd;

public:
    std::string message;
    bool open_connection() override;
    bool close_connection() override;
    std::string call(const std::string& msg) override;
    std::string read_line();
    GameServer(const std::string& host, int port)
        : host(host)
        , port(port)
    {
    }
};

GameServer* get_game_server(const std::string& host, int port)
{
    GameServer* server = new GameServer(host, port);
    bool flag = server->open_connection();
    assert(flag);
    return server;
}

bool GameServer::open_connection()
{
    WSADATA wsaData;
    sockaddr_in serverAddress;
    int result;

    // 初始化 Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }

    // 创建套接字
    this->socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == INVALID_SOCKET) {
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    // 设置服务器地址
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, this->host.c_str(), &serverAddress.sin_addr); // 设置服务器 IP 地址
    serverAddress.sin_port = htons(this->port); // 设置服务器端口号

    // 连接服务器
    result = connect(socket_fd, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if (result == SOCKET_ERROR) {
        std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl;
        closesocket(socket_fd);
        WSACleanup();
        return false;
    }

    std::cout << "Connected to server." << std::endl;
    return true;
}

bool GameServer::close_connection()
{
    int ret = -1;
    ret = shutdown(this->socket_fd, SD_BOTH);
    if (ret == SOCKET_ERROR) {
        return false;
    }
    Sleep(100);
    ret = closesocket(this->socket_fd);
    WSACleanup();
    return true;
}

std::string GameServer::call(const std::string& msg)
{
    int bytes = send(this->socket_fd, msg.c_str(), msg.size(), 0);
    send(this->socket_fd, "\n", 1, 0);
    std::string ret = this->read_line();
    return ret;
}

std::string GameServer::read_line()
{
    std::string line;
    constexpr int size = 1024;
    char buffer[size + 1];
    int bytes = 0;
    while (true) {
        bytes = recv(this->socket_fd, buffer, size, 0);
        if (bytes == -1) {
            throw "server close";
        }
        buffer[bytes] = '\0';
        std::string message = buffer;
        size_t line_end_pos = message.find('\n');
        if (line_end_pos != std::string::npos) {
            line += message.substr(0, line_end_pos);
            message = message.substr(line_end_pos + 1);
            break;
        } else {
            line += message;
        }
    }
    return line;
}
