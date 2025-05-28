#pragma once

#include <Windows.h>
#include <nlohmann/json.hpp>
extern int screen_width, screen_height;
constexpr int MAIN_WINDOW(WM_APP + 1);

struct PosOffset; // Forward declaration to avoid incomplete type issues

struct Pos {
    int w_pos;
    int h_pos;
    Pos(int w_pos, int h_pos)
        : w_pos(w_pos)
        , h_pos(h_pos)
    {
    }
    Pos() = default;
    bool operator==(const struct Pos& other) const
    {
        return this->w_pos == other.w_pos && this->h_pos == other.h_pos;
    }
    bool operator!=(const struct Pos& other) const
    {
        return !(*this == other);
    }
    bool operator<(const struct Pos& other) const
    {
        return this->h_pos < other.h_pos || (this->h_pos == other.h_pos && this->w_pos < other.w_pos);
    }
    bool operator<=(const struct Pos& other) const
    {
        return *this < other || *this == other;
    }
    bool in_area(const struct Pos& left_top, const struct Pos& right_bottom) const
    {
        return this->w_pos >= left_top.w_pos && this->w_pos <= right_bottom.w_pos
            && this->h_pos >= left_top.h_pos && this->h_pos <= right_bottom.h_pos;
    }
    friend struct PosOffset;
    Pos operator+(const struct PosOffset& pos_offset) const; // Declaration
    Pos& operator+=(const struct PosOffset& pos_offset); // Declaration
};

struct PosOffset {
    int width_offset, height_offset;
    PosOffset(int width_offset, int height_offset)
        : width_offset(width_offset)
        , height_offset(height_offset)
    {
    }
    PosOffset() = default;
};

template <typename T, size_t N>
constexpr size_t array_length(T (&)[N])
{
    return N;
}

using json = nlohmann::json;
void to_json(json& j, const Pos& pos);
void from_json(const json& j, struct Pos& pos);
