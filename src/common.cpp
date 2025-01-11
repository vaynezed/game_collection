#pragma once
#include <cstdio>
#include "common.hpp"
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
