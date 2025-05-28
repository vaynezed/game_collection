#pragma once
#include "common.hpp"
#include <cstdio>
int screen_width { -1 }, screen_height { -1 };

FILE* log_file = nullptr;
// Define the operator+ function outside the Pos struct
Pos Pos::operator+(const struct PosOffset& pos_offset) const
{
    return Pos(this->w_pos + pos_offset.width_offset, this->h_pos + pos_offset.height_offset);
}

Pos& Pos::operator+=(const PosOffset& pos_offset)
{
    *this = *this + pos_offset;
    return *this;
}

void to_json(json& j, const Pos& pos)
{
    j = json { { "w_pos", pos.w_pos }, { "h_pos", pos.h_pos } };
}

void from_json(const json& j, struct Pos& pos)
{
    j.at("w_pos").get_to(pos.w_pos);
    j.at("h_pos").get_to(pos.h_pos);
}
