
#include "game.hpp"

bool Game::is_game_init()
{
    return this->game_init_flag;
}

void Game::goback_main_menu()
{
	SendMessage(*main_hwnd_ptr, MAIN_WINDOW, NULL, NULL);
    game_cleanup();
}

std::wstring Game::to_string()
{
    return L"Game Class";
}