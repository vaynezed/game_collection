#pragma once

#include "../game.hpp"

constexpr int UNFILL = 0;
constexpr int FILL = 1;
typedef std::vector<std::vector<uint8_t>> matrix_t;

enum class color_t : int8_t {
    PURPLE = 0,
    RED = 1,
    ORANGE = 2,
    BLUE = 3,
    GREEN = 4,
    UNDEFINED = -1
};

extern const matrix_t I, O;
class Tetris : public virtual Game {
public:
    void game_init(HWND& hwnd) override;
    void game_loop() override;
    void game_cleanup() override;
    void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam) override;
    std::wstring to_string() override;
    Tetris() = default;
    ~Tetris() = default;

private:
    static const uint8_t block_height { 30 }, block_width { 30 };
    static const uint8_t wall_bmp_height { 128 }, wall_bmp_width { 128 };
    static const std::vector<matrix_t> matrixs;

    struct tetris_shape_t {
        matrix_t matrix;
        int color;
        int height_pos;
        int width_pos;
    };
    struct game_data_t {
        tetris_shape_t tetris_shape;
        int score;
        std::vector<std::vector<int>> matrix;
        bool init = false;
    };
    game_data_t game_data;

    tetris_shape_t gen_tetris_shape();
    void move_tetris(int x_offset);
    int hs, ws;
    HFONT hFont;
    HBITMAP block_bmp { nullptr }, wall_bmp { nullptr };
    ULONGLONG g_tpre { 0 }, g_tnow { 0 };
    HDC g_hdc { nullptr }, g_mdc { nullptr }, g_bufdc { nullptr };

    void clear_lines();
    void draw_wall();
    void draw_tetris();
    void draw_game_scene();
    void draw_graph();
    void draw_score();
    void restart_level();
    void continue_game();
    bool corssing(const tetris_shape_t& tetris_shape);
    void update_game_data();
    void fill_graph(const tetris_shape_t& tetris_shape);
};
