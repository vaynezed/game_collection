#pragma once
#include "base/ables.hpp"
#include <Windows.h>

class Button : public base {
    static int ID;
    static int get_id();
    static HWND id2hwnd(int id);
    static int hwnd2id(int id);
    int id;
    HWND hwnd;
    std::string to_string() override;
};
