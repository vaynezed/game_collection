#include "game.hpp"
#include <vector>

class G_2048_t : public Game {
    constexpr static int DEFAULT_WIDTH = 5;
    constexpr static int DEFAULT_HEIGHT = 5;
    constexpr static int GRID_ELE_WIDTH = 64 * 5;
    constexpr static int GRID_ELE_HEIGHT = 64;
    constexpr static int FONT_HEIGHT = GRID_ELE_HEIGHT / 3 * 2;
    constexpr static int FONT_WIDTH = FONT_HEIGHT;
    constexpr static int GRID_DATA_WIDTH = GRID_ELE_WIDTH / 3 * 2;
    constexpr static int GRID_DATA_HEIGHT = GRID_ELE_HEIGHT / 3 * 2;
    constexpr static int NO_ELE = -1;
    constexpr static int SCORE_GOAL = 2048;
    enum class direction_t : int8_t {
        none = -1,
        up = 0,
        right = 1,
        left = 2,
        down = 3
    };
    enum direction_t direction = direction_t::none;

private:
    int8_t x_offset = 0, y_offset = 0;
    int grid_height, grid_width;
    std::vector<std::vector<int>> grids;
    void game_init(HWND& hwnd) override;
    void game_loop() override;
    void game_cleanup() override;
    void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam) override;
    void update_game_data();
    void draw_game_data();
    void check_game_data();
    void draw_grid();
    void gravitate_grids();
    void merge_grids();
    void restart_level();
    void draw_grid_data();
    void random_grids_data();
    void show_dlg(int dlg_id);
    std::wstring to_string() override;
    ULONGLONG g_tpre { 0 }, g_tnow { 0 };
    HPEN pen_new { nullptr }, pen_old { nullptr };
    HFONT font_new { nullptr }, font_old { nullptr };
    HDC g_hdc { nullptr }, g_mdc { nullptr }, g_bufdc { nullptr };

public:
    G_2048_t();
    ~G_2048_t() = default;
};