#include "games/snake.hpp"
#include "games/utils.hpp"

void Snake::game_init(HWND& hwnd)
{
    this->main_hwnd_ptr = &hwnd;
    g_hdc = GetDC(hwnd);
    g_mdc = CreateCompatibleDC(g_hdc);
    g_bufdc = CreateCompatibleDC(g_hdc);

    HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);
    SelectObject(g_mdc, bmp);

    this->load_resource();
    game_init_flag = true;
}

void Snake::load_resource()
{
    this->apple_bmp = (HBITMAP)LoadImage(NULL, TEXT("./resource/snake/apple_red_64.bmp"), IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    this->bomb_bmp = (HBITMAP)LoadImage(NULL, TEXT("./resource/snake/bomb_64.bmp"), IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    this->head_bmp = (HBITMAP)LoadImage(NULL, TEXT("./resource/snake/snake_yellow_head_64.bmp"), IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    this->blob_bmp = (HBITMAP)LoadImage(NULL, TEXT("./resource/snake/snake_yellow_blob_64.bmp"), IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
    this->wall_bmp = (HBITMAP)LoadImage(NULL, TEXT("./resource/snake/wall_block_64_0.bmp"), IMAGE_BITMAP, 64, 64, LR_LOADFROMFILE);
}
void Snake::render()
{
    SelectObject(g_bufdc, this->bomb_bmp);
    TransparentBlt(g_mdc, 0, 0, 64, 64, g_bufdc, 0, 0, 64, 64, RGB(0, 0, 0));
    BitBlt(g_hdc, screen_width / 2, screen_height / 2, 64, 64, g_mdc, 0, 0, SRCCOPY);
}

void Snake::game_loop()
{
    this->t_now = GetTickCount64();

    bool need_re_rend { (this->t_now - this->t_pre >= 50) };
    if (need_re_rend) {
        game::util::draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);

        this->render();

        this->t_pre = GetTickCount64();
    }
}

void Snake::game_cleanup()
{
}

void Snake::game_process_key_down(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
}

std::string Snake::to_string()
{
    return "Ã∞≥‘…ﬂ";
}
