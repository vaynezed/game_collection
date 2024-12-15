#pragma once
#include <Windows.h>
#include <vector>

namespace game {
namespace util {

    void draw_blackground(HDC* hdc, int width, int height, int color);

    int rand_range(int lower, int upper);

    template <typename T>
    const T& rand_ele(const std::vector<T>& eles)
    {
        int pos = rand_range(0, eles.size());
        const T& t = eles[pos];
        return t;
    }
}
}
