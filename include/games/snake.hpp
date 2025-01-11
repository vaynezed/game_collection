#pragma once

#include "../game.hpp"

#include <forward_list>
#include <list>

class Snake : public Game {
private:
    HDC g_hdc { nullptr }, g_mdc { nullptr }, g_bufdc { nullptr };
    HBITMAP wall_bmp, apple_bmp, head_bmp, blob_bmp, bomb_bmp;
    ULONGLONG t_pre { 0 }, t_now { 0 };
    enum class direction_t : int8_t {
        up = 0,
        right = 1,
        left = 2,
        down = 3
    };

    void init_game_data();
    void load_resource();
    void render();
    void render_wall();
    void render_snake();
    void render_food();
    void render_bomb();
    void update_direction(direction_t new_direction);
    void fill_pos(std::list<struct Pos>& poss, int need_size);
    void game_fail();

    constexpr static int NUMBER_OF_FRAME = 60;
    constexpr static int MAX_NUMBER_OF_FOOD = 5;
    constexpr static int MAX_NUMBER_OF_BOMB = 5;
    constexpr static int RENDER_TIME_INTERVAL = 1000 / NUMBER_OF_FRAME;
    constexpr static int BMP_W { 32 }, BMP_H { 32 };

    direction_t snake_direction;

    std::list<Pos> snake_body;
    std::list<Pos> foods;
    std::list<Pos> bombs;
    int scene_w, scene_h;
    int game_map_w, game_map_h;
    int frame_count = 0;

public:
    void update_game_data();
    void game_init(HWND& hwnd) override;
    void game_loop() override;
    void game_cleanup() override;
    void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam) override;
    std::wstring to_string() override;
};