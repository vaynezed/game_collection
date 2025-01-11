#include "base/ables.hpp"
#include <string>

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
