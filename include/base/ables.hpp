
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
    T resource; // �洢��Դ
    std::function<void(T&)> release_function; // �ͷ���Դ�ĺ���

public:
    // ���캯������ʼ����Դ�����ͷź���
    RAII(T res, std::function<void(T&)> release_func)
        : resource(std::move(res))
        , release_function(std::move(release_func))
    {
    }

    // ��ֹ��������
    RAII(const RAII&) = delete;
    RAII& operator=(const RAII&) = delete;

    // ֧���ƶ�����
    RAII(RAII&& other)
    noexcept
        : resource(std::move(other.resource))
        , release_function(std::move(other.release_function))
    {
    }

    RAII& operator=(RAII&& other) noexcept
    {
        if (this != &other) {
            release(); // �ͷŵ�ǰ��Դ
            resource = std::move(other.resource);
            release_function = std::move(other.release_function);
        }
        return *this;
    }

    // �����������ͷ���Դ
    ~RAII()
    {
        release();
    }

    // �ṩ����Դ�ķ���
    T& get()
    {
        return resource;
    }

    const T& get() const
    {
        return resource;
    }

private:
    // �ͷ���Դ��˽�з���
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
