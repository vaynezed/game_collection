#include "games/tetris.hpp"
#include "games/utils.hpp"
#include <Windows.h>
#include <tchar.h>
#include <time.h>

template <typename T = int>
void rotate_image(std::vector<std::vector<T>>& src)
{
    std::vector<std::vector<T>> dst;
    int rows = src.size();
    int cols = src[0].size();

    dst.resize(cols);
    for (int i = 0; i < cols; ++i) {
        dst[i].resize(rows);
        for (int j = 0; j < rows; ++j) {
            dst[i][j] = src[j][cols - 1 - i];
        }
    }
    src = (std::move(dst));
}

const matrix_t I { { FILL, FILL, FILL, FILL } };
const matrix_t O { { FILL, FILL }, { FILL, FILL } };
const matrix_t L { { FILL, UNFILL }, { FILL, UNFILL }, { FILL, FILL } };
const matrix_t Z { { FILL, FILL, UNFILL }, { UNFILL, FILL, FILL } };
const matrix_t S { { UNFILL, FILL, FILL }, { FILL, FILL, UNFILL } };
const matrix_t J { { UNFILL, FILL }, { UNFILL, FILL }, { FILL, FILL } };
const matrix_t T {
    { FILL, FILL, FILL },
    { UNFILL, FILL, UNFILL },
};
const std::vector<matrix_t> Tetris::matrixs = { O, I, L, Z, S, J, T };

void Tetris::game_init(HWND& hwnd)
{
    // init variable
    this->hs = 30;
    this->ws = 10;
    this->g_tnow = 0;
    this->g_tpre = 0;
    this->game_init_flag = true;
    this->main_hwnd_ptr = &hwnd;
    srand(time(NULL));

    // init system resource
    this->g_hdc = GetDC(hwnd);
    g_mdc = CreateCompatibleDC(g_hdc);
    g_bufdc = CreateCompatibleDC(g_hdc);
    HBITMAP bmp = CreateCompatibleBitmap(g_hdc, screen_width, screen_height);
    SelectObject(this->g_mdc, bmp);

    // init bmp image
    const TCHAR* block_image_path = TEXT("./resource/tetris/blocks.bmp");
    this->block_bmp = (HBITMAP)LoadImage(NULL, block_image_path, IMAGE_BITMAP, 150, 30, LR_LOADFROMFILE);
    assert(this->block_bmp);
    const TCHAR* wall_image_path = TEXT("./resource/tetris/wall.bmp");
    this->wall_bmp = (HBITMAP)LoadImage(NULL, wall_image_path, IMAGE_BITMAP, 128, 128, LR_LOADFROMFILE);
    assert(this->wall_bmp);

    // init game data
    this->game_data.tetris_shape = gen_tetris_shape();
    this->game_data.matrix.resize(this->hs, std::vector<int>(this->ws, static_cast<int>(color_t::UNDEFINED)));
}

void Tetris::draw_game_scene()
{
    game::util::draw_blackground(&this->g_mdc, screen_width, screen_height, WHITE_BRUSH);

    this->draw_wall();
    this->draw_tetris();
    this->draw_graph();
    this->draw_score();

    int GRAPH_HEIGHT { block_height * this->hs };
    int GRAPH_WIDTH { block_width * (this->ws + 2) };
    BitBlt(this->g_hdc, screen_width / 2 - GRAPH_WIDTH / 2, screen_height / 2 - GRAPH_HEIGHT / 2, screen_width, screen_height, this->g_mdc, 0, 0, SRCCOPY);
}

void Tetris::game_loop()
{
    g_tnow = GetTickCount64();
    bool need_re_rend { g_tnow - g_tpre >= 500 };
    if (need_re_rend) {

        this->update_game_data();
        this->draw_game_scene();

        g_tpre = GetTickCount64();
    }
}

void Tetris::draw_score()
{
    SetTextColor(this->g_mdc, RGB(0, 0, 0)); // 设置文本颜色为黑色
    SelectObject(this->g_mdc, this->hFont);

    int height_pos = 0;
    constexpr int width_offset = 50;
    int width_pos = (this->block_width) * (this->ws + 2) + width_offset;

    TCHAR buf[1024];
    _stprintf_s(buf, TEXT("得分为 %d"), this->game_data.score);
    TextOut(this->g_mdc, width_pos, height_pos, buf, lstrlen(buf));
}

// 画地图数据
void Tetris::draw_graph()
{
    SelectObject(g_bufdc, this->block_bmp);
    for (int h = 0; h < this->hs; h++) {
        for (int w = 0; w < this->ws; w++) {
            int color = this->game_data.matrix[h][w];
            if (color == static_cast<int>(color_t::UNDEFINED)) {
                continue;
            }
            int h_pos = h * this->block_height;
            int w_pos = w * this->block_width + block_width;
            int color_bmp_width_offset = color * this->block_width;
            TransparentBlt(this->g_mdc, w_pos, h_pos, this->block_width, this->block_height, this->g_bufdc, color_bmp_width_offset, 0, this->block_width, this->block_height, RGB(0, 0, 0));
        }
    }
}

// 判断方块和地图数据是否相交
bool Tetris::corssing(const tetris_shape_t& tetris_shape)
{
    const matrix_t& matrix = tetris_shape.matrix;
    int width = matrix.front().size();
    int height = matrix.size();
    for (int w_pos = 0; w_pos < width; w_pos++) {
        for (int h_pos = 0; h_pos < height; h_pos++) {
            int target_w_pos = tetris_shape.width_pos + w_pos;
            int target_h_pos = tetris_shape.height_pos + h_pos;
            int ele = matrix[h_pos][w_pos];
            int target_ele = this->game_data.matrix[target_h_pos][target_w_pos];
            if (ele == FILL && target_ele != static_cast<int>(color_t::UNDEFINED)) {
                return true;
            }
        }
    }
    return false;
}

// 将俄罗斯方块填充到地图数据中
void Tetris::fill_graph(const tetris_shape_t& tetris_shape)
{
    const matrix_t& matrix = tetris_shape.matrix;
    int width = matrix.front().size();
    int height = matrix.size();
    for (int w_pos = 0; w_pos < width; w_pos++) {
        for (int h_pos = 0; h_pos < height; h_pos++) {
            int target_w_pos = tetris_shape.width_pos + w_pos;
            int target_h_pos = tetris_shape.height_pos + h_pos;
            int ele_flag = matrix[h_pos][w_pos];
            if (ele_flag == FILL) {
                this->game_data.matrix[target_h_pos][target_w_pos] = tetris_shape.color;
            }
        }
    }
}

void Tetris::update_game_data()
{
    this->clear_lines();

    tetris_shape_t tetris_shape = this->game_data.tetris_shape;
    tetris_shape.height_pos++;

    int shape_height = tetris_shape.matrix.size();
    int shape_bottom_pos = shape_height + (tetris_shape.height_pos);
    bool corssing_wall_flag = (shape_bottom_pos > this->hs);

    if (corssing_wall_flag || this->corssing(tetris_shape)) {
        this->fill_graph(this->game_data.tetris_shape);
        tetris_shape_t gen_tetris_shape = this->gen_tetris_shape();

        if (this->corssing(gen_tetris_shape) == true) {
            HINSTANCE hInstance
                = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
            int btn_id = DialogBox(hInstance, MAKEINTRESOURCE(IDD_VICTORY),
                *main_hwnd_ptr, default_dlg_wnd_proc);
            if (btn_id == ID_GOBACK) {
                goback_main_menu();
            } else if (btn_id == ID_RESTART_BTN) {
                restart_level();
            }
        } else {
            this->game_data.tetris_shape = gen_tetris_shape;
        }
        return;
    }

    this->game_data.tetris_shape.height_pos++;
}

// 生成新的俄罗斯方块
Tetris::tetris_shape_t Tetris::gen_tetris_shape()
{
    tetris_shape_t tetris_shape;
    tetris_shape.color = (game::util::rand_range(0, 4));
    tetris_shape.matrix = game::util::rand_ele(Tetris::matrixs);
    tetris_shape.width_pos = (this->ws) / 2;
    tetris_shape.height_pos = 0;
    return tetris_shape;
}

void Tetris::restart_level()
{

    game_data_t game_data;
    game_data.score = 0;
    game_data.init = true;
    game_data.tetris_shape = this->gen_tetris_shape();
    game_data.matrix.resize(this->hs, std::vector<int>(this->ws, static_cast<int>(color_t::UNDEFINED)));

    this->game_data = game_data;
}

void Tetris::move_tetris(int x_offset)
{
    tetris_shape_t& tetris_shape = this->game_data.tetris_shape;
    matrix_t& matrix = tetris_shape.matrix;
    int matrix_width = static_cast<int>(matrix.front().size());

    int& width_pos = this->game_data.tetris_shape.width_pos;
    int expected_left_pos = width_pos + x_offset;
    int expected_right_pos = width_pos + matrix_width + x_offset;
    bool crossing_wall_flag = (expected_left_pos < 0 || expected_right_pos > this->ws);

    tetris_shape_t expected_tetris = this->game_data.tetris_shape;
    expected_tetris.width_pos = expected_left_pos;

    if (crossing_wall_flag || this->corssing(expected_tetris)) {
        return;
    } else {
        width_pos = expected_left_pos;
    }
}

void Tetris::draw_wall()
{
    SelectObject(g_bufdc, this->wall_bmp);
    // left  wall
    for (int h = 0; h < hs + 1; h++) {
        int w_pos = 0;
        int h_pos = h * this->block_height;
        TransparentBlt(this->g_mdc, w_pos, h_pos, this->block_width, this->block_height, this->g_bufdc, 0, 0, wall_bmp_width, wall_bmp_width, RGB(0, 0, 0));
    }
    // right wall
    for (int h = 0; h < hs + 1; h++) {
        int h_pos = h * this->block_height;
        int w_pos = (this->ws + 1) * (this->block_width);
        TransparentBlt(this->g_mdc, w_pos, h_pos, this->block_width, this->block_height, this->g_bufdc, 0, 0, wall_bmp_width, wall_bmp_height, RGB(0, 0, 0));
    }
    // bottom wall
    for (int w = 0; w < ws + 2; w++) {
        int w_pos = w * this->block_width;
        int h_pos = this->hs * this->block_height;
        TransparentBlt(this->g_mdc, w_pos, h_pos, this->block_width, this->block_height, this->g_bufdc, 0, 0, wall_bmp_width, wall_bmp_height, RGB(0, 0, 0));
    }
}
void Tetris::draw_tetris()
{
    SelectObject(g_bufdc, this->block_bmp);
    const tetris_shape_t& tetris_shape = this->game_data.tetris_shape;
    matrix_t matrix = tetris_shape.matrix;
    int color = tetris_shape.color;
    int width = matrix.front().size();
    int height = matrix.size();
    int color_bmp_width_offset = color * this->block_width;
    for (int w_pos = 0; w_pos < width; w_pos++) {
        for (int h_pos = 0; h_pos < height; h_pos++) {
            int bmp_width_pos = (tetris_shape.width_pos + w_pos + 1) * this->block_width;
            int bmp_height_pos = (tetris_shape.height_pos + h_pos) * this->block_height;
            int ele = matrix[h_pos][w_pos];
            if (ele == FILL) {
                TransparentBlt(this->g_mdc, bmp_width_pos, bmp_height_pos, this->block_width, this->block_height, this->g_bufdc, color_bmp_width_offset, 0, this->block_width, this->block_height, RGB(0, 0, 0));
            }
        }
    }
}

void Tetris::clear_lines()
{
    using namespace std;
    vector<int> lines;
    vector<vector<int>>& matrix = this->game_data.matrix;
    int height = matrix.size();
    int width = matrix.front().size();

    for (int height_pos = 0; height_pos < height; height_pos++) {
        bool all_fill_flag = true;
        for (int width_pos = 0; width_pos < width; width_pos++) {
            int ele = matrix[height_pos][width_pos];
            if (ele == static_cast<int>(color_t::UNDEFINED)) {
                all_fill_flag = false;
                break;
            }
        }
        if (all_fill_flag) {
            lines.push_back(height_pos);
        }
    }

    this->game_data.score += lines.size();
    int lines_size = lines.size();

    for (int line = lines.size() - 1; line >= 0; line--) {
        int offset = lines[line];
        matrix.erase(matrix.begin() + offset);
    }

    if (lines.size() > 0) {
        for (int idx = 0; idx < lines.size(); idx++) {
            vector<int> v;
            v.resize(this->ws, static_cast<int>(color_t::UNDEFINED));
            matrix.insert(matrix.begin(), v);
        }
    }
}
void Tetris::game_cleanup()
{
    game_data = {};
    this->game_init_flag = false;
    DeleteObject(this->block_bmp);
    DeleteObject(this->wall_bmp);
    DeleteDC(this->g_bufdc);
    DeleteDC(this->g_mdc);
    DeleteObject(this->hFont);
    ReleaseDC(*main_hwnd_ptr, g_hdc);
    main_hwnd_ptr = nullptr;
}

void Tetris::continue_game()
{
}

void Tetris::game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam)
{
    int x_offset { 0 };

    HINSTANCE hInstance
        = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
    switch (wParam) {
    case VK_ESCAPE: {
        int ret = DialogBox(hInstance, MAKEINTRESOURCE(ID_RESTART_DLG),
            *main_hwnd_ptr, default_dlg_wnd_proc);
        if (ret == ID_GOBACK) {
            goback_main_menu();
        } else if (ret == ID_RESTART_BTN) {
            restart_level();
        } else if (ret == IDC_CONTINUE) {
            this->continue_game();
        }
        break;
    }
    case VK_UP:
    case 'D':
        x_offset += 1;
        move_tetris(x_offset);
        break;
    case VK_LEFT:
    case 'A':
        x_offset -= 1;
        move_tetris(x_offset);
        break;
    case 'S': {
        tetris_shape_t tetris_shape = this->game_data.tetris_shape;
        while (true) {
            this->game_data.tetris_shape = tetris_shape;
            tetris_shape.height_pos++;

            int shape_height = tetris_shape.matrix.size();
            int shape_bottom_pos = shape_height + (tetris_shape.height_pos);
            bool corssing_wall_flag = (shape_bottom_pos > this->hs);

            if (corssing_wall_flag || this->corssing(tetris_shape)) {
                this->fill_graph(this->game_data.tetris_shape);
                tetris_shape_t gen_tetris_shape = this->gen_tetris_shape();

                if (this->corssing(gen_tetris_shape) == true) {
                    HINSTANCE hInstance
                        = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
                    int btn_id = DialogBox(hInstance, MAKEINTRESOURCE(IDD_VICTORY),
                        *main_hwnd_ptr, default_dlg_wnd_proc);
                    if (btn_id == ID_GOBACK) {
                        goback_main_menu();
                    } else if (btn_id == ID_RESTART_BTN) {
                        restart_level();
                    }
                } else {
                    this->game_data.tetris_shape = gen_tetris_shape;
                }
                return;
            }
        }
        break;
    }
    case 'R': {
        tetris_shape_t expected_tetris = this->game_data.tetris_shape;
        rotate_image(expected_tetris.matrix);
        if (this->corssing(expected_tetris) == false) {
            this->game_data.tetris_shape = expected_tetris;
        }
        break;
    }
    default:
        break;
    }
}
std::wstring Tetris::to_string()
{
    return std::wstring(L"俄罗斯方块");
}
