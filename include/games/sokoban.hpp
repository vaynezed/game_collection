#pragma once
#include "../game.hpp"

class SokoBanGame : public Game {
public:
    void game_init(HWND& hwnd) override;
    void game_loop() override;
    void game_cleanup() override;
    void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam) override;
    std::string to_string() override;
    SokoBanGame();
    ~SokoBanGame();

private:
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

    bool game_end { false };
    struct game_data_t game_data {
    };

private:
    graph_data_t& get_graph_data();
    void update_graph(int x_offset, int y_offset);
    void restart_level();
    void goback_main_menu();
    void find_person_from_graph(graph_data_t& graph_data);
    void init_game_data();
    void load_resource();
    void init_system_resource(HWND& hwnd);
    void draw_blackground(HDC* hdc, int width, int height, int color = WHITE_BRUSH);
    void draw_blackground();
    void draw_ele(int pic_type, int y, int x);
    bool is_game_finish();
    void next_level();
    void pre_level();
    void draw_graph();
    void check_game_data();
};
