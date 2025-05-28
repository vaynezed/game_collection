
#include "nlohmann/json.hpp"
#include <cstdarg>
#include <iostream>
#include <sstream>
#include <string>

class base {
public:
    virtual std::wstring to_string() = 0;
};

template <typename T>
std::wstring str(const T& ele)
{
    std::wstringstream ss;
    ss << ele;
    return ss.str();
}

extern std::string format(const char* fmt, ...);

#include <functional>
#include <iostream>
#include <utility>

template <typename T>
class RAII {
private:
    T resource; // 存储资源
    std::function<void(T&)> release_function; // 释放资源的函数

public:
    // 构造函数：初始化资源并绑定释放函数
    RAII(T res, std::function<void(T&)> release_func)
        : resource(std::move(res))
        , release_function(std::move(release_func))
    {
    }

    // 禁止复制语义
    RAII(const RAII&) = delete;
    RAII& operator=(const RAII&) = delete;

    // 支持移动语义
    RAII(RAII&& other)
    noexcept
        : resource(std::move(other.resource))
        , release_function(std::move(other.release_function))
    {
    }

    RAII& operator=(RAII&& other) noexcept
    {
        if (this != &other) {
            release(); // 释放当前资源
            resource = std::move(other.resource);
            release_function = std::move(other.release_function);
        }
        return *this;
    }

    // 析构函数：释放资源
    ~RAII()
    {
        release();
    }

    // 提供对资源的访问
    T& get()
    {
        return resource;
    }

    const T& get() const
    {
        return resource;
    }

private:
    // 释放资源的私有方法
    void release()
    {
        if (release_function) {
            release_function(resource);
            std::cout << "Resource released: " << resource << std::endl;
        }
    }
};

class Server {
public:
    virtual bool open_connection() = 0;
    virtual bool close_connection() = 0;
    virtual std::string call(const std::string& msg) = 0;
};
class GameServer;

GameServer* get_game_server(const std::string& host, int port);
