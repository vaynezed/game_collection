#include "game.hpp"
//-----------------------------------【Game_Init(
//)函数】--------------------------------------
//	描述：初始化函数，进行一些简单的初始化
//------------------------------------------------------------------------------------------------
HDC g_hdc, g_mdc, g_bufdc;
HBITMAP b_male, b_box, b_ball, b_wall;
int male_pic_width, male_pic_height;
int male_sprite_width, male_sprit_height;

int x { -1 }, y { -1 };
HWND* main_hwnd { nullptr };
bool game_init_flag { false };
constexpr int MAIN_WINDOW(WM_APP + 1);

enum class character_status_t : char { UP = 2,
    LEFT = 3,
    RIGHT = 1,
    DOWN = 0 };

character_status_t character_status;
int character_idx;
void game_process_key_down(HWND hwnd, UINT message, WPARAM wparam,
    LPARAM lparam)
{
    switch (wparam) {
    case VK_ESCAPE:
        game_cleanup();
        SendMessage(hwnd, MAIN_WINDOW, 0, 0);
        break;
    case VK_UP:
    case 'W':
        character_status = character_status_t::UP;
        y--;
        break;
    case VK_DOWN:
    case 'S':
        character_status = character_status_t::DOWN;
        y++;
        break;
    case VK_RIGHT:
    case 'D':
        character_status = character_status_t::RIGHT;
        x++;
        break;
    case VK_LEFT:
    case 'A':
        character_status = character_status_t::LEFT;
        x--;
        break;
    default:
        break;
    }
}
BOOL game_init(HWND hwnd)
{
    game_init_flag = true;
    x = 0;
    y = 0;

    main_hwnd = &hwnd;

    int screen_width, screen_height;
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);

    g_hdc = GetDC(hwnd);
    g_mdc = CreateCompatibleDC(g_hdc);
    g_bufdc = CreateCompatibleDC(g_hdc);
    HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);

    SelectObject(g_mdc, bmp);

    ::male_pic_width = 64 * 5;
    ::male_pic_height = 51 * 5;
    ::male_sprite_width = ::male_pic_width / 4;
    ::male_sprit_height = ::male_pic_height / 3;
    b_male = (HBITMAP)LoadImage(NULL, TEXT("./resource/male.bmp"), IMAGE_BITMAP,
        ::male_pic_width, ::male_pic_height, LR_LOADFROMFILE);

    b_box = (HBITMAP)LoadImage(NULL, TEXT("./resource/box.bmp"), IMAGE_BITMAP,
        60, 60, LR_LOADFROMFILE);

    b_ball = (HBITMAP)LoadImage(NULL, TEXT("./resource/ball.bmp"), IMAGE_BITMAP,
        32, 32, LR_LOADFROMFILE);
    b_wall = (HBITMAP)LoadImage(NULL, TEXT("./resource/wall.bmp"), IMAGE_BITMAP,
        128, 128, LR_LOADFROMFILE);
    return TRUE;
}

//-----------------------------------【Game_Paint(
//)函数】--------------------------------------
//	描述：绘制函数，在此函数中进行绘制操作
//--------------------------------------------------------------------------------------------------
VOID game_paint(HWND hwnd, ULONGLONG* pre_paint_time)
{
    int screen_width, screen_height;
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);

    HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    RECT rect { 0, 0, screen_width, screen_height };
    FillRect(g_mdc, &rect, hBrush);
    DeleteObject(hBrush);

    SelectObject(g_bufdc, b_male);
    TransparentBlt(g_mdc, x * 64, y * 64, 64, 64,
        g_bufdc, static_cast<int>(character_status) * male_sprite_width, character_idx / 5 * male_sprit_height, male_sprite_width, male_sprit_height,
        RGB(0, 0, 0));
    character_idx = (character_idx + 1) % 15;

    SelectObject(g_bufdc, b_box);
    BitBlt(g_mdc, 10 * 64, 10 * 64, 64, 64, g_bufdc, 0, 0, SRCCOPY);

    SelectObject(g_bufdc, b_ball);
    TransparentBlt(g_mdc, 20 * 64, 20 * 64, 64, 64,
        g_bufdc, 0, 0, 32, 32,
        RGB(0, 0, 0));

    SelectObject(g_bufdc, b_wall);
    BitBlt(g_mdc, 15 * 64, 15 * 64, 64, 64,
        g_bufdc, 0, 0, SRCCOPY);

    BitBlt(g_hdc, 0, 0, screen_width, screen_height, g_mdc, 0, 0, SRCCOPY);
    *pre_paint_time = GetTickCount64();
}

//-----------------------------------【Game_CleanUp(
//)函数】--------------------------------
//	描述：资源清理函数，在此函数中进行程序退出前资源的清理工作
//---------------------------------------------------------------------------------------------------
BOOL game_cleanup()
{
    game_init_flag = false;
    DeleteObject(b_male);
    DeleteObject(b_ball);
    DeleteObject(b_wall);
    DeleteObject(b_box);
    DeleteDC(g_bufdc);
    DeleteDC(g_mdc);
    ReleaseDC(*main_hwnd, g_hdc);
    main_hwnd = nullptr;
    return TRUE;
}