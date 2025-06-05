
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
void Game::show_dlg(UINT DLG_ID)
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(*main_hwnd_ptr, GWLP_HINSTANCE);
    int ret = DialogBox(hInstance, MAKEINTRESOURCE(DLG_ID),
        *main_hwnd_ptr, default_dlg_wnd_proc);
    if (ret == ID_GOBACK) {
        goback_main_menu();
    } else if (ret == ID_RESTART_BTN) {
        HWND* main_hwnd_ptr = this->main_hwnd_ptr;
        game_cleanup();
        game_init(*main_hwnd_ptr);
    }
}

std::wstring Game::to_string()
{
    return L"Game Class";
}

std::vector<std::wstring> Game::game_models()
{
    return std::vector<std::wstring>();
}

void Game::set_game_model(int idx)
{
}

Game::~Game()
{
}

