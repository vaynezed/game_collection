#include "game.hpp"
#include "common.hpp"
#include "../resource.h"
#include <vector>

constexpr int NONE = 0, wall = 1, person = 2, ball = 4, box = 8;
constexpr int sprite_height { 64 }, sprite_width { 64 };
constexpr int male_pic_width{ 64 }, male_pic_height{ 51};
constexpr int male_sprite_width{ male_pic_width / 4 }, male_sprit_height{male_pic_height / 3};


struct graph_data_t {
    std::vector<std::vector<int>> graph{};
    int male_x{ -1 }, male_y{ -1 };
};
enum class character_status_t : char { UP = 2,
    LEFT = 3,
    RIGHT = 1,
    DOWN = 0 };


ULONGLONG g_tpre{ 0 }, g_tnow{ 0 };
HDC g_hdc{ nullptr }, g_mdc{ nullptr }, g_bufdc{ nullptr };
HBITMAP b_male{ nullptr }, b_box{ nullptr }, b_ball{ nullptr }, b_wall{ nullptr };
HWND* main_hwnd { nullptr };

character_status_t character_status{character_status_t::UP};
int character_idx{ 0 };

bool game_init_flag { false };
bool game_end{ false };
graph_data_t graph_data{};

void update_graph(int x_offset, int y_offset);

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
    case 'R':
        break;
    default:
        break;
    }
}

void init_game_data() {
    graph_data.graph = { { wall, wall, wall, wall, wall },
        { wall, person, box, ball, wall },
        { wall, wall, wall, wall, wall } };
    graph_data.male_y = graph_data.male_x = 1;
    game_init_flag = true;

}
void load_resource() {
    b_male = (HBITMAP)LoadImage(NULL, TEXT("./resource/male.bmp"), IMAGE_BITMAP,
        ::male_pic_width, ::male_pic_height, LR_LOADFROMFILE);

    b_box = (HBITMAP)LoadImage(NULL, TEXT("./resource/box.bmp"), IMAGE_BITMAP,
        60, 60, LR_LOADFROMFILE);

    b_ball = (HBITMAP)LoadImage(NULL, TEXT("./resource/ball.bmp"), IMAGE_BITMAP,
        32, 32, LR_LOADFROMFILE);
    b_wall = (HBITMAP)LoadImage(NULL, TEXT("./resource/wall.bmp"), IMAGE_BITMAP,
        128, 128, LR_LOADFROMFILE);
}

void init_system_resource(HWND & hwnd) {
    g_hdc = GetDC(hwnd);
    g_mdc = CreateCompatibleDC(g_hdc);
    g_bufdc = CreateCompatibleDC(g_hdc);
    HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);
    ::main_hwnd = &hwnd;

    SelectObject(g_mdc, bmp);
}

BOOL game_init(HWND& hwnd)
{
    init_game_data();
    init_system_resource(hwnd);
    load_resource();
    return TRUE;
}

void draw_blackground()
{
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
        TransparentBlt(g_mdc, x * sprite_height, y * sprite_width, sprite_height, sprite_width,
            g_bufdc, static_cast<int>(character_status) * male_sprite_width, character_idx / 5 * male_sprit_height, male_sprite_width, male_sprit_height,
            RGB(0, 0, 0));
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
// 自定义消息值
#define WM_MY_CUSTOM_MESSAGE (WM_USER + 1)

INT_PTR CALLBACK MyDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        // 处理命令消息，例如按钮点击
        if (LOWORD(wParam) == IDOK)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;



    default:
        return (INT_PTR)FALSE;
    }
    return (INT_PTR)FALSE;
}

void check_game_data() {
    std::vector<std::vector<int>>& graph = graph_data.graph;
    int Y = graph.size(), X = graph.front().size();
    bool find_ball_flag{ false };
	for (int y = 0; y < Y; y++) {
		for (int x = 0; x < X; x++) {
            int ele = graph[y][x];
            if (ele == ball) {
                find_ball_flag = true;
                break;
            }
        }
        if (find_ball_flag) {
            break;
        }
    }
    if (!find_ball_flag) {
        //DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), *main_hwnd, MyDialogProc);
    }
}

VOID game_loop()
{
    ::g_tnow = GetTickCount64();
    bool need_re_rend{ (::g_tnow - ::g_tpre >= 50) };
    if (need_re_rend) {
		draw_blackground();
		draw_graph();
        check_game_data();
        g_tpre = GetTickCount64();
    }
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