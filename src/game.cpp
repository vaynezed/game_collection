#include "game.hpp"
#include "common.hpp"
#include <vector>

HDC g_hdc, g_mdc, g_bufdc;
HBITMAP b_male, b_box, b_ball, b_wall;
int male_pic_width, male_pic_height;
int male_sprite_width, male_sprit_height;

HWND* main_hwnd { nullptr };
bool game_init_flag { false };
constexpr int MAIN_WINDOW(WM_APP + 1);

enum class character_status_t : char { UP = 2,
    LEFT = 3,
    RIGHT = 1,
    DOWN = 0 };

constexpr int NONE = 0, wall = 1, person = 2, ball = 4, box = 8;

character_status_t character_status;
int character_idx;
constexpr int sprite_height { 64 }, sprite_width { 64 };

struct graph_data_t {
    std::vector<std::vector<int>> graph;
    int male_x, male_y;
};
graph_data_t graph_data;

void update_graph(int x_offset, int y_offset);

bool is_gme_end()
{
    return false;
}

void update_graph(int x_offset, int y_offset)
{
    int& person = graph_data.graph[graph_data.male_y][graph_data.male_x];

    int next_male_x = graph_data.male_x + x_offset;
    int next_male_y = graph_data.male_y + y_offset;
    int& ele = graph_data.graph[next_male_y][next_male_x];

    if (ele & wall) {
        return;
    } else if (ele & box) {
        int next_ele_x = next_male_x + x_offset;
        int next_ele_y = next_male_y + y_offset;
        int& next_ele = graph_data.graph[next_ele_y][next_ele_x];
        if ((next_ele & wall) || (next_ele & box)) {
            return;
        } else {
            graph_data.male_x = next_male_x;
            graph_data.male_y = next_male_y;
            ele |= person;
            person &= (~person);
            next_ele |= box;
            ele &= (~box);
        }
    } else {
        ele |= person;
        person &= (~person);
        graph_data.male_x = next_male_x;
        graph_data.male_y = next_male_y;
    }
}
void game_process_key_down(HWND hwnd, UINT message, WPARAM wparam,
    LPARAM lparam)
{
    int x_offset { 0 }, y_offset { 0 };
    switch (wparam) {
    case VK_ESCAPE:
        game_cleanup();
        SendMessage(hwnd, MAIN_WINDOW, 0, 0);
        break;
    case VK_UP:
    case 'W':
        character_status = character_status_t::UP;
        y_offset = 1;
        update_graph(x_offset, y_offset);
        break;
    case VK_DOWN:
    case 'S':
        character_status = character_status_t::DOWN;
        x_offset = -1;
        update_graph(x_offset, y_offset);
        break;
    case VK_RIGHT:
    case 'D':
        character_status = character_status_t::RIGHT;
        x_offset = 1;
        update_graph(x_offset, y_offset);
        break;
    case VK_LEFT:
    case 'A':
        character_status = character_status_t::LEFT;
        x_offset = -1;
        update_graph(x_offset, y_offset);
        break;
    default:
        break;
    }
}

BOOL game_init(HWND hwnd)
{
    graph_data.graph = { { wall, wall, wall, wall, wall },
        { wall, person, box, ball, wall },
        { wall, wall, wall, wall, wall } };
    graph_data.male_y = graph_data.male_x = 1;
    game_init_flag = true;

    main_hwnd = &hwnd;

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

void draw_blackground()
{
    screen_width = GetSystemMetrics(SM_CXSCREEN);
    screen_height = GetSystemMetrics(SM_CYSCREEN);

    HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    RECT rect { 0, 0, screen_width, screen_height };
    FillRect(g_mdc, &rect, hBrush);
    DeleteObject(hBrush);
}

void draw_ele(int pic_type, int y, int x)
{
    if (pic_type & wall) {
        SelectObject(g_bufdc, b_wall);
        BitBlt(g_mdc, x * sprite_height, y * sprite_width, sprite_height, sprite_width,
            g_bufdc, 0, 0, SRCCOPY);
    } else if (pic_type & person) {
        SelectObject(g_bufdc, b_male);
        BOOL success_flag = TransparentBlt(g_mdc, x * sprite_height, y * sprite_width, sprite_height, sprite_width,
            g_bufdc, static_cast<int>(character_status) * male_sprite_width, character_idx / 5 * male_sprit_height, male_sprite_width, male_sprit_height,
            RGB(0, 0, 0));
        if (!success_flag)
            MessageBeep(NULL);
    } else if (pic_type & box) {
        SelectObject(g_bufdc, b_box);
        BitBlt(g_mdc, x * sprite_height, y * sprite_width, sprite_height, sprite_width, g_bufdc, 0, 0, SRCCOPY);
    } else if (pic_type & ball) {
        SelectObject(g_bufdc, b_ball);
        TransparentBlt(g_mdc, x * sprite_height, y * sprite_width, sprite_height, sprite_width,
            g_bufdc, 0, 0, 32, 32,
            RGB(0, 0, 0));
    }
}

void draw_graph()
{
    std::vector<std::vector<int>>& graph = graph_data.graph;

    int X = graph.front().size(), Y = graph.size();
    for (int y = 0; y < Y; y++) {
        for (int x = 0; x < X; x++) {
            draw_ele(graph[y][x], y, x);
        }
    }
    character_idx = (character_idx + 1) % 15;
    const int GRAPH_HEIGHT { Y * sprite_height }, GRAPH_WIDTH { X * sprite_width };
    BitBlt(g_hdc, screen_width / 2 - GRAPH_WIDTH / 2, screen_height / 2 - GRAPH_HEIGHT / 2, GRAPH_WIDTH * 5, GRAPH_HEIGHT * 5, g_mdc, 0, 0, SRCCOPY);
}

VOID game_update(HWND hwnd, ULONGLONG* pre_paint_time)
{
    draw_blackground();
    draw_graph();
    *pre_paint_time = GetTickCount64();
}

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