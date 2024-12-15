#pragma once

#include "../game.hpp"

class Snake : public Game {
private:
    HWND* main_hwnd_ptr;
    HDC g_hdc { nullptr }, g_mdc { nullptr }, g_bufdc { nullptr };
    HBITMAP wall_bmp, apple_bmp, head_bmp, blob_bmp, bomb_bmp;
    ULONG t_pre { 0 }, t_now { 0 };
    void load_resource();
    void render();

public:
    void game_init(HWND& hwnd) override;
    void game_loop() override;
    void game_cleanup() override;
    void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam) override;
    std::string to_string() override;
};