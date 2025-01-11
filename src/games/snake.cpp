#include "games/snake.hpp"
#include "games/utils.hpp"
#include <assert.h>
#include <set>
#include <unordered_set>

/*
贪吃蛇的思路
1. 构建蛇的数据结构。
2. 移动蛇
3. 做检测

*/

void Snake::update_game_data()
{
    Pos new_snake_head = this->snake_body.front();
    if (this->snake_direction == direction_t::up) {
        new_snake_head.h_pos--;
    } else if (this->snake_direction == direction_t::down) {
        new_snake_head.h_pos++;
    } else if (this->snake_direction == direction_t::left) {
        new_snake_head.w_pos--;
    } else if (this->snake_direction == direction_t::right) {
        new_snake_head.w_pos++;
    }

    bool in_wall = (new_snake_head.w_pos >= 0 && new_snake_head.w_pos < this->game_map_w && new_snake_head.h_pos >= 0 && new_snake_head.h_pos < this->game_map_h);
    if (in_wall == false) {
        this->game_fail();
    } else {
        bool next_is_food = false;
        bool next_is_bomb = false;
        bool next_is_snake = false;
        std::list<Pos>::iterator food_it = std::find(this->foods.begin(), this->foods.end(), new_snake_head);
        std::list<Pos>::iterator bomb_it = std::find(this->bombs.begin(), this->bombs.end(), new_snake_head);
        std::list<Pos>::iterator snake_it = std::find(this->snake_body.begin(), this->snake_body.end(), new_snake_head);
        next_is_food = food_it != this->foods.end();
        next_is_bomb = bomb_it != this->bombs.end();
        next_is_snake = snake_it != this->snake_body.end();

        if (next_is_snake || next_is_bomb) {
            this->game_fail();
        } else if (next_is_food == false && next_is_bomb == false) {
            this->snake_body.push_front(new_snake_head);
            this->snake_body.pop_back();
        } else if (next_is_food) {
            this->foods.erase(food_it);
            this->snake_body.push_front(new_snake_head);
            if (this->foods.empty()) {
                this->bombs.clear();
                fill_pos(this->bombs, MAX_NUMBER_OF_BOMB);
                fill_pos(this->foods, MAX_NUMBER_OF_FOOD);
            }
        }
    }
}

void Snake::game_init(HWND& hwnd)
{
    this->main_hwnd_ptr = &hwnd;
    g_hdc = GetDC(hwnd);
    g_mdc = CreateCompatibleDC(g_hdc);
    g_bufdc = CreateCompatibleDC(g_hdc);

    HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);
    SelectObject(g_mdc, bmp);
    DeleteObject(bmp);

    this->load_resource();
    game_init_flag = true;

    this->init_game_data();
}

void Snake::render_wall()
{
    const int scene_width = this->BMP_W * (this->scene_w);
    const int scene_height = this->BMP_H * (this->scene_h);

    constexpr int wall_top = 0;
    const int wall_bottom = scene_height - BMP_H;
    constexpr int wall_left = 0;
    const int wall_right = scene_width - this->BMP_W;

    // draw wall
    SelectObject(g_bufdc, this->wall_bmp);
    // top
    for (int w_idx = 0; w_idx < this->scene_w; w_idx++) {
        TransparentBlt(g_mdc, Snake::BMP_W * w_idx, wall_top, Snake::BMP_W, Snake::BMP_H, g_bufdc, 0, 0, 64, 64, RGB(0, 0, 0));
    }

    // bottom
    for (int w_idx = 0; w_idx < this->scene_w; w_idx++) {
        TransparentBlt(g_mdc, Snake::BMP_W * w_idx, wall_bottom, Snake::BMP_W, Snake::BMP_H, g_bufdc, 0, 0, 64, 64, RGB(0, 0, 0));
    }

    // left
    for (int h_idx = 0; h_idx < this->game_map_h; h_idx++) {
        TransparentBlt(g_mdc, 0, Snake::BMP_H * (h_idx + 1), Snake::BMP_W, Snake::BMP_H, g_bufdc, 0, 0, 64, 64, RGB(0, 0, 0));
    }

    // right
    for (int h_idx = 0; h_idx < this->game_map_h; h_idx++) {
        TransparentBlt(g_mdc, wall_right, Snake::BMP_H * (h_idx + 1), Snake::BMP_W, Snake::BMP_H, g_bufdc, 0, 0, 64, 64, RGB(0, 0, 0));
    }
}

void Snake::render_snake()
{
    SelectObject(this->g_bufdc, this->head_bmp);
    std::list<Pos>::iterator it = this->snake_body.begin();

    // draw head
    const Pos& head_pos = *it;
    SelectObject(this->g_bufdc, this->head_bmp);
    TransparentBlt(g_mdc, (head_pos.w_pos + 1) * (this->BMP_W), (head_pos.h_pos + 1) * this->BMP_H, Snake::BMP_W, Snake::BMP_H, g_bufdc, 0, 0, 64, 64, RGB(0, 0, 0));

    ++it;

    // draw body
    SelectObject(this->g_bufdc, this->blob_bmp);
    std::list<Pos>::iterator end_it = this->snake_body.end();
    for (; it != end_it; it++) {
        const Pos& body_pos = *it;
        TransparentBlt(g_mdc, (body_pos.w_pos + 1) * (this->BMP_W), (body_pos.h_pos + 1) * this->BMP_H, Snake::BMP_W, Snake::BMP_H, g_bufdc, 0, 0, 64, 64, RGB(0, 0, 0));
    }
}

void Snake::render_food()
{
    SelectObject(this->g_bufdc, this->apple_bmp);

    // draw food
    std::list<Pos>::iterator it = this->foods.begin();
    std::list<Pos>::iterator end_it = this->foods.end();
    for (; it != end_it; it++) {
        const Pos& body_pos = *it;
        TransparentBlt(g_mdc, (body_pos.w_pos + 1) * (this->BMP_W), (body_pos.h_pos + 1) * this->BMP_H, Snake::BMP_W, Snake::BMP_H, g_bufdc, 0, 0, 64, 64, RGB(0, 0, 0));
    }
}

void Snake::render_bomb()
{
    SelectObject(this->g_bufdc, this->bomb_bmp);

    // draw food
    std::list<Pos>::iterator it = this->bombs.begin();
    std::list<Pos>::iterator end_it = this->bombs.end();
    for (; it != end_it; it++) {
        const Pos& body_pos = *it;
        TransparentBlt(g_mdc, (body_pos.w_pos + 1) * (this->BMP_W), (body_pos.h_pos + 1) * this->BMP_H, Snake::BMP_W, Snake::BMP_H, g_bufdc, 0, 0, 64, 64, RGB(0, 0, 0));
    }
}

void Snake::update_direction(direction_t new_snake_direction)
{
    int old_direction = static_cast<int>(this->snake_direction);
    int new_direction = static_cast<int>(new_snake_direction);
    if (old_direction + new_direction != 3) {
        this->snake_direction = new_snake_direction;
    }
}

void Snake::fill_pos(std::list<Pos>& poss, int need_size)
{
    std::set<Pos> pos_set(poss.begin(), poss.end());
    while (pos_set.size() < need_size) {
        Pos pos;
        pos.w_pos = game::util::rand_range(1, this->game_map_w);
        pos.h_pos = game::util::rand_range(1, this->game_map_h);

        bool in_snake = false;
        for (const Pos& snake_body : this->snake_body) {
            if (snake_body == pos) {
                in_snake = true;
                break;
            }
        }
        if (in_snake == false) {
            bool in_set = false;
            in_set = pos_set.count(pos) > 0;
            if (in_set == false) {
                pos_set.insert(pos);
            }
        }
    }
    poss.clear();
    for (const Pos& pos : pos_set) {
        poss.push_back(pos);
    }
}

void Snake::game_fail()
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
    int btn_id = DialogBox(hInstance, MAKEINTRESOURCE(IDD_VICTORY), *main_hwnd_ptr, default_dlg_wnd_proc);
    if (btn_id == ID_GOBACK) {
        goback_main_menu();
    } else if (btn_id == ID_RESTART_BTN) {
        this->init_game_data();
    }
}

void Snake::init_game_data()
{
    this->game_map_h = 30;
    this->game_map_w = 50;
    this->scene_h = this->game_map_h + 2;
    this->scene_w = this->game_map_w + 2;

    this->snake_direction = direction_t::up;

    Pos head, body;
    head.h_pos = game::util::rand_range(10, this->game_map_h);
    head.w_pos = game::util::rand_range(10, this->game_map_w);
    body = head;
    body.h_pos++;

    this->snake_body.clear();
    this->snake_body.push_back(head);
    this->snake_body.push_back(body);

    this->foods.clear();
    this->bombs.clear();
    fill_pos(this->foods, MAX_NUMBER_OF_FOOD);
    fill_pos(this->bombs, MAX_NUMBER_OF_BOMB);
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
    const int scene_width = this->BMP_W * (this->scene_w);
    const int scene_height = this->BMP_H * (this->scene_h);

    this->render_wall();
    this->render_snake();
    this->render_food();
    this->render_bomb();

    BitBlt(g_hdc, (screen_width - scene_width) / 2, (screen_height - scene_height) / 2, scene_width, scene_height, g_mdc, 0, 0, SRCCOPY);
}

void Snake::game_loop()
{
    this->t_now = GetTickCount64();
    bool need_re_rend { (this->t_now - this->t_pre >= Snake::RENDER_TIME_INTERVAL) };
    if (need_re_rend) {
        game::util::draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);

        this->render();
        this->t_pre = this->t_now;
        if (this->frame_count >= 20) {
            this->update_game_data();
            this->frame_count = 0;
        } else {
            this->frame_count++;
        }
    }
}

void Snake::game_cleanup()
{
    this->game_init_flag = false;

    DeleteObject(this->apple_bmp);
    DeleteObject(this->head_bmp);
    DeleteObject(this->wall_bmp);
    DeleteObject(this->blob_bmp);
    DeleteObject(this->bomb_bmp);

    DeleteDC(this->g_bufdc);
    DeleteDC(this->g_mdc);
    ReleaseDC(*main_hwnd_ptr, g_hdc);

    this->main_hwnd_ptr = nullptr;
}

std::wstring Snake::to_string()
{
    return L"贪吃蛇";
}

void Snake::game_process_key_down(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(*this->main_hwnd_ptr, GWLP_HINSTANCE);
    switch (wparam) {
    case VK_ESCAPE: {
        INT_PTR ret = DialogBox(hInstance, MAKEINTRESOURCE(ID_RESTART_DLG), *main_hwnd_ptr, default_dlg_wnd_proc);
        if (ret == ID_GOBACK) {
            this->goback_main_menu();
        } else if (ret == ID_RESTART_BTN) {
        }
        break;
    }
    case VK_UP:
    case 'W': {
        direction_t new_snake_direction = direction_t::up;
        this->update_direction(new_snake_direction);
        break;
    }
    case VK_DOWN:
    case 'S': {
        direction_t new_snake_direction = direction_t::down;
        this->update_direction(new_snake_direction);
        break;
    }
    case 'A': {
        direction_t new_snake_direction = direction_t::left;
        this->update_direction(new_snake_direction);
        break;
    }
    case 'D': {
        direction_t new_snake_direction = direction_t::right;
        this->update_direction(new_snake_direction);
        break;
    }
    default:
        break;
    }
}
