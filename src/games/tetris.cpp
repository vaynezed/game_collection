#include "games/tetris.hpp"
#include <Windows.h>

const graph_t I { { FILL, FILL, FILL, FILL } };
const graph_t O { { FILL, FILL }, { FILL, FILL } };

void Tetris::game_init(HWND& hwnd)
{
    g_hdc = GetDC(hwnd);
    g_mdc = CreateCompatibleDC(g_hdc);
    g_bufdc = CreateCompatibleDC(g_hdc);
    HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);

    main_hwnd_ptr = &hwnd;
    SelectObject(this->g_mdc, bmp);

    this->game_init_flag = true;
    const TCHAR* block_image_path = TEXT("./resource/tetris/blocks.bmp");
    this->block_bmp = (HBITMAP)LoadImage(NULL, block_image_path, IMAGE_BITMAP, 150, 30, LR_LOADFROMFILE);
    this->g_tnow = 0;
    this->g_tpre = 0;
    assert(this->block_bmp);
}
void draw_blackground(HDC* hdc, int width, int height, int color)
{
    HBRUSH hBrush = (HBRUSH)GetStockObject(color);
    RECT rect { 0, 0, width, height };
    FillRect(*hdc, &rect, hBrush);
    DeleteObject(hBrush);
}

void Tetris::game_loop()
{
    g_tnow = GetTickCount64();
    bool need_re_rend { g_tnow - g_tpre >= 50 };
    if (need_re_rend) {
        draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);

        SelectObject(g_bufdc, this->block_bmp);
        BitBlt(this->g_mdc, 0, 0, 150, 30, this->g_bufdc, 0, 0, SRCCOPY);
        BitBlt(this->g_hdc, 0, 0, screen_width, screen_height, this->g_mdc, 0, 0, SRCCOPY);
        g_tpre = GetTickCount64();
    }
}
void Tetris::game_cleanup()
{
}
void Tetris::game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam)
{
}
std::string Tetris::to_string()
{
    return std::string("¶íÂÞË¹·½¿é");
}
