#include "games/g_2048.hpp"
#include "common.hpp"
#include "games/utils.hpp"
#include <random>
#include <tchar.h>

void G_2048_t::game_init(HWND& hwnd)
{
    this->grids.resize(this->grid_height, std::vector<int>(this->grid_width, NO_ELE));
    g_hdc = GetDC(hwnd);
    this->g_mdc = CreateCompatibleDC(g_hdc);
    this->g_bufdc = CreateCompatibleDC(g_hdc);
    HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);
    main_hwnd_ptr = &hwnd;
    SelectObject(g_mdc, bmp);
    DeleteObject(bmp);
    SetTextColor(this->g_mdc, RGB(0, 0, 0));
    game::util::draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);
    {
        this->pen_new = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        this->pen_old = (HPEN)(SelectObject(this->g_mdc, this->pen_new));
    }
    {

        int pixelsPerInchY = GetDeviceCaps(this->g_mdc, LOGPIXELSY);
        int pixelsPerInchX = GetDeviceCaps(this->g_mdc, LOGPIXELSX);
        int heightInLogicUnits = -MulDiv(FONT_HEIGHT, 72, pixelsPerInchY);
        int widthInLogicUnits = MulDiv(FONT_WIDTH, 72, pixelsPerInchX);

        this->font_new = CreateFont(
            heightInLogicUnits, // 字体高度（逻辑单位）
            widthInLogicUnits, // 字体宽度
            0, // 字体方向
            0, // 字体倾斜角度
            FW_BOLD, // 字体重量（粗体）
            FALSE, // 是否斜体
            FALSE, // 是否下划线
            FALSE, // 是否删除线
            ANSI_CHARSET, // 字符集
            OUT_DEFAULT_PRECIS, // 输出精度
            CLIP_DEFAULT_PRECIS, // 剪切精度
            DEFAULT_QUALITY, // 字体质量
            DEFAULT_PITCH | FF_DONTCARE, // 字符间距和字体族
            L"Arial" // 字体名称
        );
        this->font_old = (HFONT)SelectObject(g_mdc, this->font_new);
    }
    this->game_init_flag = true;
    this->random_grids_data();
}

void G_2048_t::game_loop()
{
    g_tnow = GetTickCount64();
    bool need_re_rend { g_tnow - g_tpre >= 500 };
    //need_re_rend = (direction != direction_t::none);
    if (need_re_rend) {
        g_tpre = GetTickCount64();
        this->update_game_data();
        this->draw_game_data();
        direction = direction_t::none;
        check_game_data();
    }
}

void G_2048_t::game_cleanup()
{
    this->grids.clear();
    this->game_init_flag = false;
    SelectObject(this->g_mdc, pen_old);
    DeleteObject(this->pen_new);
    SelectObject(this->g_mdc, font_old);
    DeleteObject(this->font_new);
    DeleteDC(this->g_mdc);
    DeleteDC(this->g_bufdc);
    ReleaseDC(*main_hwnd_ptr, g_hdc);
    main_hwnd_ptr = nullptr;
}

void G_2048_t::game_process_key_down(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
    case VK_ESCAPE: {
        this->show_dlg(ID_RESTART_DLG);
        break;
    }
    case 'A':
    case VK_LEFT:
        direction = direction_t::left;
        x_offset = -1;
        y_offset = 0;
        break;
    case 'D':
    case VK_RIGHT:
        direction = direction_t::right;
        x_offset = 1;
        y_offset = 0;
        break;
    case 'W':
    case VK_UP:
        y_offset = -1;
        x_offset = 0;
        direction = direction_t::up;
        break;
    case 'S':
    case VK_DOWN:
        y_offset = 1;
        x_offset = 0;
        direction = direction_t::down;
        break;
    default:
        break;
    }
}

void G_2048_t::update_game_data()
{
    if (direction != direction_t::none) {
        this->gravitate_grids();
        this->merge_grids();
        this->gravitate_grids();
        this->random_grids_data();
    }
}

void G_2048_t::draw_game_data()
{
    game::util::draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);
    this->draw_grid();
    this->draw_grid_data();
    const int GRID_WIDTH { static_cast<int>(this->grids.size()) * G_2048_t::GRID_ELE_WIDTH };
    const int GRID_HEIGHT { static_cast<int>(this->grids.front().size()) * G_2048_t::GRID_ELE_HEIGHT };
    BitBlt(this->g_hdc, screen_width / 2 - GRID_WIDTH / 2, screen_height / 2 - GRID_HEIGHT / 2, screen_width, screen_height, this->g_mdc, 0, 0, SRCCOPY);
}

void G_2048_t::check_game_data()
{
    bool has_none_flag = false;
    bool score_goal_flag = false;
    const int H = static_cast<int>(this->grids.size());
    const int W = static_cast<int>(this->grids.front().size());
    for (int h_idx = 0; h_idx < H; ++h_idx) {
        for (int w_idx = 0; w_idx < W; ++w_idx) {
            int ele = this->grids[h_idx][w_idx];
            if (ele == SCORE_GOAL) {
                score_goal_flag = true;
                break;
            } else if (ele == NO_ELE) {
                has_none_flag = true;
                break;
            }
        }
        if (score_goal_flag) {
            break;
        }
    }
    if (score_goal_flag == true) {
        this->show_dlg(IDD_VICTORY);
    } else if (has_none_flag == false) {
        this->show_dlg(ID_RESTART_DLG);
    }
}

void G_2048_t::draw_grid()
{
    const int height = static_cast<int>(this->grids.size()) + 1;
    const int line_widht = static_cast<int>(this->grids.front().size()) * G_2048_t::GRID_ELE_WIDTH;
    for (int h_idx = 0; h_idx < height; ++h_idx) {
        const int h_pos = h_idx * G_2048_t::GRID_ELE_HEIGHT;
        MoveToEx(this->g_mdc, 0, h_pos, NULL);
        LineTo(this->g_mdc, line_widht, h_pos);
    }

    const int width = static_cast<int>(this->grids.front().size()) + 1;
    const int line_height = static_cast<int>(this->grids.size()) * G_2048_t::GRID_ELE_HEIGHT;
    for (int w_idx = 0; w_idx < width; ++w_idx) {
        const int w_pos = w_idx * G_2048_t::GRID_ELE_WIDTH;
        MoveToEx(this->g_mdc, w_pos, 0, NULL);
        LineTo(this->g_mdc, w_pos, line_height);
    }
}

void G_2048_t::gravitate_grids()
{
    const int W = this->grids.front().size();
    const int H = this->grids.size();
    if (direction == direction_t::left) {
        bool flag = true;
        while (flag) {
            flag = false;
            for (int w_idx = 1; w_idx < W; ++w_idx) {
                for (int h_idx = 0; h_idx < H; ++h_idx) {
                    int& cur_ele = this->grids[h_idx][w_idx];
                    int& next_ele = this->grids[h_idx + y_offset][w_idx + x_offset];
                    if (cur_ele != NO_ELE && next_ele == NO_ELE) {
                        std::swap(cur_ele, next_ele);
                        flag = true;
                    }
                }
            }
        }
    } else if (direction == direction_t::right) {
        bool flag = true;
        while (flag) {
            flag = false;
            for (int w_idx = W - 2; w_idx >= 0; --w_idx) {
                for (int h_idx = 0; h_idx < H; ++h_idx) {
                    int& cur_ele = this->grids[h_idx][w_idx];
                    int& next_ele = this->grids[h_idx + y_offset][w_idx + x_offset];
                    if (cur_ele != NO_ELE && next_ele == NO_ELE) {
                        std::swap(cur_ele, next_ele);
                        flag = true;
                    }
                }
            }
        }
    } else if (direction == direction_t::up) {
        bool flag = true;
        while (flag) {
            flag = false;
            for (int h_idx = 1; h_idx < H; ++h_idx) {
                for (int w_idx = 0; w_idx < W; ++w_idx) {
                    int& cur_ele = this->grids[h_idx][w_idx];
                    int& next_ele = this->grids[h_idx + y_offset][w_idx + x_offset];
                    if (cur_ele != NO_ELE && next_ele == NO_ELE) {
                        std::swap(cur_ele, next_ele);
                        flag = true;
                    }
                }
            }
        }
    } else if (direction == direction_t::down) {
        bool flag = true;
        while (flag) {
            flag = false;
            for (int h_idx = H - 2; h_idx >= 0; --h_idx) {
                for (int w_idx = 0; w_idx < W; ++w_idx) {
                    int& cur_ele = this->grids[h_idx][w_idx];
                    int& next_ele = this->grids[h_idx + y_offset][w_idx + x_offset];
                    if (cur_ele != NO_ELE && next_ele == NO_ELE) {
                        std::swap(cur_ele, next_ele);
                        flag = true;
                    }
                }
            }
        }
    }
}

void G_2048_t::merge_grids()
{
    const int W = this->grids.front().size();
    const int H = this->grids.size();
    if (direction == direction_t::left) {
        for (int w_idx = 1; w_idx < W; ++w_idx) {
            for (int h_idx = 0; h_idx < H; ++h_idx) {
                int& cur_ele = this->grids[h_idx][w_idx];
                int& next_ele = this->grids[h_idx + y_offset][w_idx + x_offset];
                if (cur_ele != NO_ELE && next_ele == cur_ele) {
                    next_ele += cur_ele;
                    cur_ele = NO_ELE;
                }
            }
        }
    } else if (direction == direction_t::right) {
        for (int w_idx = W - 2; w_idx >= 0; --w_idx) {
            for (int h_idx = 0; h_idx < H; ++h_idx) {
                int& cur_ele = this->grids[h_idx][w_idx];
                int& next_ele = this->grids[h_idx + y_offset][w_idx + x_offset];
                if (cur_ele != NO_ELE && next_ele == cur_ele) {
                    next_ele += cur_ele;
                    cur_ele = NO_ELE;
                }
            }
        }
    } else if (direction == direction_t::up) {
        for (int h_idx = 1; h_idx < H; ++h_idx) {
            for (int w_idx = 0; w_idx < W; ++w_idx) {
                int& cur_ele = this->grids[h_idx][w_idx];
                int& next_ele = this->grids[h_idx + y_offset][w_idx + x_offset];
                if (cur_ele != NO_ELE && next_ele == cur_ele) {
                    next_ele += cur_ele;
                    cur_ele = NO_ELE;
                }
            }
        }
    } else if (direction == direction_t::down) {
        for (int h_idx = H - 2; h_idx >= 0; --h_idx) {
            for (int w_idx = 0; w_idx < W; ++w_idx) {
                int& cur_ele = this->grids[h_idx][w_idx];
                int& next_ele = this->grids[h_idx + y_offset][w_idx + x_offset];
                if (cur_ele != NO_ELE && next_ele == cur_ele) {
                    next_ele += cur_ele;
                    cur_ele = NO_ELE;
                }
            }
        }
    }
}

void G_2048_t::restart_level()
{
    game::util::draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);
    this->grids.clear();
    this->grids.resize(grid_width, std::vector<int>(grid_height, -1));
    this->random_grids_data();
}

void G_2048_t::draw_grid_data()
{
    const int height = static_cast<int>(this->grids.size());
    const int width = static_cast<int>(this->grids.front().size());
    const int w_center_pos = (GRID_ELE_WIDTH / 2) - ((GRID_DATA_WIDTH) / 2);
    const int h_center_pos = (GRID_ELE_HEIGHT / 2) - ((GRID_DATA_HEIGHT) / 2);
    for (int h_idx = 0; h_idx < height; ++h_idx) {
        for (int w_idx = 0; w_idx < width; ++w_idx) {
            const int ele = this->grids[h_idx][w_idx];
            if (ele != -1) {
                const int h_pos = (h_idx * GRID_ELE_HEIGHT) + h_center_pos;
                const int w_pos = (w_idx * GRID_ELE_WIDTH) + w_center_pos;
                RECT rect { w_pos, h_pos, w_pos + GRID_DATA_WIDTH, h_pos + GRID_DATA_HEIGHT };
                TCHAR buf[16];
                _stprintf_s(buf, TEXT("%d"), ele);
                DrawText(this->g_mdc, buf, -1, &rect, DT_CENTER | DT_VCENTER | DT_WORDBREAK);
            }
        }
    }
}

void G_2048_t::random_grids_data()
{
    int size = this->grid_height * this->grid_width;
    int rand_num = game::util::rand_range(0, size);
    const int H = this->grid_height;
    const int W = this->grid_width;
    int count = 0;
    for (int h_idx = 0; h_idx < H; ++h_idx) {
        for (int w_idx = 0; w_idx < W; ++w_idx) {
            const int ele = this->grids[h_idx][w_idx];
            if (ele == NO_ELE) {
                if (count == rand_num) {
                    this->grids[h_idx][w_idx] = 2;
                    return;
                }
                ++count;
            }
        }
    }
}

void G_2048_t::show_dlg(int DLG_ID)
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
    int ret = DialogBox(hInstance, MAKEINTRESOURCE(DLG_ID),
        *main_hwnd_ptr, default_dlg_wnd_proc);
    if (ret == ID_GOBACK) {
        goback_main_menu();
    } else if (ret == ID_RESTART_BTN) {
        restart_level();
    }
}

std::wstring G_2048_t::to_string()
{
    return std::wstring(L"G2048");
}

G_2048_t::G_2048_t()
{
    this->grid_height = DEFAULT_HEIGHT;
    this->grid_width = DEFAULT_WIDTH;
}
