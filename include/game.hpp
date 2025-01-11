#pragma once
#include <Windows.h>
#include <cassert>
#include <vector>

#include "../resource.h"
#include "base/ables.hpp"
#include "common.hpp"
#include "dlg.hpp"

class Game : public virtual base {
protected:
    bool game_init_flag { false };
    HWND* main_hwnd_ptr;

public:
    virtual void game_init(HWND& hwnd) = 0;
    virtual void game_loop() = 0;
    virtual void game_cleanup() = 0;
    virtual bool is_game_init();
    virtual void game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
        LPARAM lParam)
        = 0;
    virtual void goback_main_menu();
    virtual void show_dlg(UINT DLG_ID);
    std::wstring to_string() override;
    virtual std::vector<std::wstring> game_models();
    virtual void set_game_model(int idx);

    virtual ~Game();
};

constexpr int NONE = 0, WALL = 1, PERSON = 2, BALL = 4, BOX = 8;
constexpr int sprite_height { 64 }, sprite_width { 64 };
constexpr int male_pic_width { 64 }, male_pic_height { 51 };
constexpr int male_sprite_width { male_pic_width / 4 },
    male_sprit_height { male_pic_height / 3 };
