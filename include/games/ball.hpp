#pragma once

#include "game.hpp"

class Pool : public Game {
public:
	void game_init(HWND& hwnd) override;
	void game_loop() override;
	void game_cleanup() override;
	std::wstring to_string() override;
	std::vector<std::wstring> game_models() override;
	void set_game_model(int idx) override;
};
