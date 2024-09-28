#include "game.hpp"
#include "../resource.h"
#include "common.hpp"
#include "dlg.hpp"
#include <cassert>
#include <vector>

constexpr int NONE = 0, WALL = 1, PERSON = 2, BALL = 4, BOX = 8;
constexpr int sprite_height { 64 }, sprite_width { 64 };
constexpr int male_pic_width { 64 }, male_pic_height { 51 };
constexpr int male_sprite_width { male_pic_width / 4 },
    male_sprit_height { male_pic_height / 3 };

struct graph_data_t {
    std::vector<std::vector<int>> graph {};
    int male_x { -1 }, male_y { -1 };
};

struct game_data_t {
    std::vector<struct graph_data_t> graphs;
    int current_graph_idx { 0 };
    struct graph_data_t current_graph;
};
enum class character_status_t : char {
    UP = 2,
    LEFT = 3,
    RIGHT = 1,
    DOWN = 0
};

ULONGLONG g_tpre { 0 }, g_tnow { 0 };
HDC g_hdc { nullptr }, g_mdc { nullptr }, g_bufdc { nullptr };
HBITMAP b_male { nullptr }, b_box { nullptr }, b_ball { nullptr },
    b_wall { nullptr };
HWND* main_hwnd_ptr { nullptr };

character_status_t character_status { character_status_t::UP };
int character_idx { 0 };

bool game_init_flag { false };
bool game_end { false };
struct game_data_t game_data {
};

bool is_game_init()
{
    return ::game_init_flag;
}

graph_data_t&
get_graph_data()
{
    return game_data.current_graph;
}

void update_graph(int x_offset, int y_offset)
{
    graph_data_t& graph_data = get_graph_data();
    int& PERSON = graph_data.graph[graph_data.male_y][graph_data.male_x];

    int next_male_x = graph_data.male_x + x_offset;
    int next_male_y = graph_data.male_y + y_offset;
    int& ele = graph_data.graph[next_male_y][next_male_x];

    if (ele & WALL) {
        return;
    } else if (ele & BOX) {
        int next_ele_x = next_male_x + x_offset;
        int next_ele_y = next_male_y + y_offset;
        int& next_ele = graph_data.graph[next_ele_y][next_ele_x];
        if ((next_ele & WALL) || (next_ele & BOX)) {
            return;
        } else {
            graph_data.male_x = next_male_x;
            graph_data.male_y = next_male_y;
            ele |= PERSON;
            PERSON &= (~PERSON);
            next_ele |= BOX;
            ele &= (~BOX);
        }
    } else {
        ele |= PERSON;
        PERSON &= (~PERSON);
        graph_data.male_x = next_male_x;
        graph_data.male_y = next_male_y;
    }
}

void restart_level()
{
    game_data.current_graph = game_data.graphs[game_data.current_graph_idx];
}

void goback_main_menu()
{
    SendMessage(*main_hwnd_ptr, MAIN_WINDOW, NULL, NULL);
    game_cleanup();
}
void game_process_key_down(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    int x_offset { 0 }, y_offset { 0 };

    HINSTANCE hInstance
        = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
    switch (wparam) {
    case VK_ESCAPE: {
        int ret = DialogBox(hInstance, MAKEINTRESOURCE(ID_RESTART_DLG),
            *main_hwnd_ptr, restart_dlg_wnd_proc);
        if (ret == ID_GOBACK) {
            goback_main_menu();
        } else if (ret == ID_RESTART_BTN) {
            restart_level();
        }
        break;
    }
    case VK_UP:
    case 'W':
        character_status = character_status_t::UP;
        y_offset = -1;
        update_graph(x_offset, y_offset);
        break;
    case VK_DOWN:
    case 'S':
        character_status = character_status_t::DOWN;
        y_offset = +1;
        update_graph(x_offset, y_offset);
        break;
    case VK_RIGHT:
    case 'D':
        character_status = character_status_t::RIGHT;
        x_offset = +1;
        update_graph(x_offset, y_offset);
        break;
    case VK_LEFT:
    case 'A':
        character_status = character_status_t::LEFT;
        x_offset = -1;
        update_graph(x_offset, y_offset);
        break;
    case 'R':
        break;
    default:
        break;
    }
}

void find_person_from_graph(graph_data_t& graph_data)
{
    bool find_flag = { false };
    std::vector<std::vector<int>>& graph = graph_data.graph;
    int Y = graph.size(), X = graph.front().size();
    for (int y = 0; y < Y; y++) {
        for (int x = 0; x < X; x++) {
            int ele = graph[y][x];
            if (ele == PERSON) {
                graph_data.male_x = x;
                graph_data.male_y = y;
                find_flag = true;
                break;
            }
        }
        if (find_flag) {
            break;
        }
    }
}

void init_game_data()
{
    game_init_flag = true;

    graph_data_t graph_data;
    graph_data.graph = { { WALL, WALL, WALL, WALL, WALL },
        { WALL, PERSON, BOX, BALL, WALL },
        { WALL, WALL, WALL, WALL, WALL } };
    find_person_from_graph(graph_data);
    game_data.graphs.push_back(graph_data);

    graph_data.graph = { {
                             WALL,
                             WALL,
                             WALL,
                         },
        { WALL, BALL, WALL },
        { WALL, BOX, WALL },
        { WALL, PERSON, WALL },
        { WALL, BOX, WALL },
        { WALL, BALL, WALL },
        { WALL, WALL, WALL } };
    find_person_from_graph(graph_data);
    game_data.graphs.push_back(graph_data);

    graph_data.graph = {
        { NONE, NONE, WALL, WALL, WALL, NONE, NONE },
        { NONE, NONE, WALL, BALL, WALL, NONE, NONE },
        { WALL, WALL, WALL, BOX, WALL, WALL, WALL },
        { WALL, BALL, BOX, PERSON, BOX, BALL, WALL },
        { WALL, WALL, WALL, BOX, WALL, WALL, WALL },
        { NONE, NONE, WALL, BALL, WALL, NONE, NONE },
        { NONE, NONE, WALL, WALL, WALL, NONE, NONE },
    };
    find_person_from_graph(graph_data);
    game_data.graphs.push_back(graph_data);

    graph_data.graph = {
        { NONE, NONE, NONE, WALL, WALL, WALL },
        { WALL, WALL, WALL, WALL, BALL, WALL },
        { WALL, PERSON, BOX, NONE, NONE, WALL },
        { WALL, WALL, WALL, NONE, NONE, WALL },
        { NONE, NONE, WALL, WALL, WALL, WALL },
        { NONE, NONE, NONE, NONE, NONE, NONE },
    };
    find_person_from_graph(graph_data);
    game_data.graphs.push_back(graph_data);

    game_data.current_graph_idx = 0;
    game_data.current_graph = game_data.graphs[game_data.current_graph_idx];
}
void load_resource()
{
    b_male = (HBITMAP)LoadImage(NULL, TEXT("./resource/male.bmp"),
        IMAGE_BITMAP, ::male_pic_width,
        ::male_pic_height, LR_LOADFROMFILE);

    b_box = (HBITMAP)LoadImage(NULL, TEXT("./resource/box.bmp"), IMAGE_BITMAP,
        60, 60, LR_LOADFROMFILE);

    b_ball = (HBITMAP)LoadImage(NULL, TEXT("./resource/ball.bmp"),
        IMAGE_BITMAP, 32, 32, LR_LOADFROMFILE);
    b_wall = (HBITMAP)LoadImage(NULL, TEXT("./resource/wall.bmp"),
        IMAGE_BITMAP, 128, 128, LR_LOADFROMFILE);
}

void init_system_resource(HWND& hwnd)
{
    g_hdc = GetDC(hwnd);
    g_mdc = CreateCompatibleDC(g_hdc);
    g_bufdc = CreateCompatibleDC(g_hdc);
    HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);
    ::main_hwnd_ptr = &hwnd;

    SelectObject(g_mdc, bmp);
}

BOOL game_init(HWND& hwnd)
{
    init_game_data();
    init_system_resource(hwnd);
    load_resource();
    return TRUE;
}

void draw_blackground(HDC* hdc, int width, int height, int color = WHITE_BRUSH)
{
    HBRUSH hBrush = (HBRUSH)GetStockObject(color);
    RECT rect { 0, 0, width, height };
    FillRect(*hdc, &rect, hBrush);
    DeleteObject(hBrush);
}

void draw_blackground()
{
    draw_blackground(&g_mdc, screen_width, screen_height, WHITE_BRUSH);
}

void draw_ele(int pic_type, int y, int x)
{
    if (pic_type & WALL) {
        SelectObject(g_bufdc, b_wall);
        BitBlt(g_mdc, x * sprite_height, y * sprite_width, sprite_height,
            sprite_width, g_bufdc, 0, 0, SRCCOPY);
    } else if (pic_type & PERSON) {
        SelectObject(g_bufdc, b_male);
        TransparentBlt(g_mdc, x * sprite_height, y * sprite_width,
            sprite_height, sprite_width, g_bufdc,
            static_cast<int>(character_status) * male_sprite_width,
            character_idx / 5 * male_sprit_height, male_sprite_width,
            male_sprit_height, RGB(0, 0, 0));
    } else if (pic_type & BOX) {
        SelectObject(g_bufdc, b_box);
        BitBlt(g_mdc, x * sprite_height, y * sprite_width, sprite_height,
            sprite_width, g_bufdc, 0, 0, SRCCOPY);
    } else if (pic_type & BALL) {
        SelectObject(g_bufdc, b_ball);
        TransparentBlt(g_mdc, x * sprite_height, y * sprite_width,
            sprite_height, sprite_width, g_bufdc, 0, 0, 32, 32,
            RGB(0, 0, 0));
    }
}

bool is_game_finish()
{
    return game_data.current_graph_idx + 1
        >= static_cast<int>(game_data.graphs.size());
}

void next_level()
{
    assert(is_game_finish() == false);
    draw_blackground(&g_hdc, screen_width, screen_height);
    ++game_data.current_graph_idx;
    game_data.current_graph = game_data.graphs[game_data.current_graph_idx];
}
void pre_level()
{
    assert(game_data.current_graph_idx > 0);
    draw_blackground(&g_hdc, screen_width, screen_height);
    --game_data.current_graph_idx;
    game_data.current_graph = game_data.graphs[game_data.current_graph_idx];
}

void draw_graph()
{
    std::vector<std::vector<int>>& graph = get_graph_data().graph;

    int X = graph.front().size(), Y = graph.size();
    for (int y = 0; y < Y; y++) {
        for (int x = 0; x < X; x++) {
            draw_ele(graph[y][x], y, x);
        }
    }
    character_idx = (character_idx + 1) % 15;
    const int GRAPH_HEIGHT { Y * sprite_height }, GRAPH_WIDTH { X * sprite_width };
    BitBlt(g_hdc, screen_width / 2 - GRAPH_WIDTH / 2,
        screen_height / 2 - GRAPH_HEIGHT / 2, GRAPH_WIDTH * 5,
        GRAPH_HEIGHT * 5, g_mdc, 0, 0, SRCCOPY);
}

void check_game_data()
{
    std::vector<std::vector<int>>& graph = get_graph_data().graph;
    int Y = graph.size(), X = graph.front().size();
    bool find_ball_flag { false };
    for (int y = 0; y < Y; y++) {
        for (int x = 0; x < X; x++) {
            int ele = graph[y][x];
            if (ele == BALL) {
                find_ball_flag = true;
                break;
            }
        }
        if (find_ball_flag) {
            break;
        }
    }
    HINSTANCE hInstance
        = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
    if (!find_ball_flag) {
        bool game_finish_flag = is_game_finish();
        if (game_finish_flag == false) {
            int ret = DialogBox(hInstance, MAKEINTRESOURCE(ID_NEXT_LEVEL_DLG),
                *main_hwnd_ptr, next_level_dlg_wnd_proc);
            if (ret == ID_GOBACK) {
                goback_main_menu();
            } else if (ret == ID_NEXT_LEVEL) {
                next_level();
            }
        } else {
            int ret = DialogBox(hInstance, MAKEINTRESOURCE(ID_GAME_FINISH_DLG),
                *main_hwnd_ptr, pre_level_dlg_wnd_proc);
            if (ret == ID_GOBACK) {
                goback_main_menu();
            } else if (ret == ID_PRE_LEVEL) {
                pre_level();
            }
        }
    }
}

VOID game_loop()
{
    ::g_tnow = GetTickCount64();
    bool need_re_rend { (::g_tnow - ::g_tpre >= 50) };
    if (need_re_rend) {
        draw_blackground();
        draw_graph();
        check_game_data();
        g_tpre = GetTickCount64();
    }
}

BOOL game_cleanup()
{
    game_data = {};
    game_init_flag = false;
    DeleteObject(b_male);
    DeleteObject(b_ball);
    DeleteObject(b_wall);
    DeleteObject(b_box);
    DeleteDC(g_bufdc);
    DeleteDC(g_mdc);
    ReleaseDC(*main_hwnd_ptr, g_hdc);
    main_hwnd_ptr = nullptr;
    return TRUE;
}