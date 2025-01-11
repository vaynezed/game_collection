#pragma once

#include <Windows.h>
extern int screen_width, screen_height;
constexpr int MAIN_WINDOW(WM_APP + 1);

struct Pos {
    int w_pos;
    int h_pos;
    bool operator == (const struct Pos& other) const {
        return this->w_pos == other.w_pos && this->h_pos == other.h_pos;
    }
    bool operator <(const struct Pos& other) const {
        return this->h_pos < other.h_pos || (this->h_pos == other.h_pos && this->w_pos < other.w_pos);
    }
};


extern FILE* log_file;
