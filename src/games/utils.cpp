#include "./games/utils.hpp"

void draw_blackground(HDC* hdc, int width, int height, int color)
{
    HBRUSH hBrush = (HBRUSH)GetStockObject(color);
    RECT rect { 0, 0, width, height };
    FillRect(*hdc, &rect, hBrush);
    DeleteObject(hBrush);
}

int rand_range(int lower, int upper)
{
    int ans { (rand() % (upper - lower)) + (lower) };
    return ans;
}