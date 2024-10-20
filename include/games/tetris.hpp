#pragma once

#include "../game.hpp"

template <typename T = int>
void rotate_image(std::vector<std::vector<T>> src)
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

constexpr int FILL = 1;
typedef std::vector<std::vector<uint8_t>> graph_t;

extern const graph_t I, O;
class Tetris : public virtual Game {
public:
    void game_init(HWND& hwnd) override;
    void game_loop() override;
    void game_cleanup() override;
    void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam) override;
    std::string to_string() override;
    Tetris() = default;
    ~Tetris() = default;

private:
    HBITMAP block_bmp { nullptr };
    ULONGLONG g_tpre { 0 }, g_tnow { 0 };
    HDC g_hdc { nullptr }, g_mdc { nullptr }, g_bufdc { nullptr };
    HWND* main_hwnd_ptr { nullptr };
};
