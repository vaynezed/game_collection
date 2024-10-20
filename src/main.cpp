#include <tchar.h>
#include "common.hpp"
#include "games/games.hpp"
#include <CommCtrl.h>
#include <memory>
#include <windows.h>
#define LEN(A) (sizeof(A) / sizeof(A[0]))

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")

const TCHAR* WINDOW_TITLE = TEXT("游戏集合");
const TCHAR* WIN_CLS = TEXT("MainWin");

constexpr int ERR { -1 }, OK { 0 };
HWND main_button, exit_button, game_combobox;
constexpr int main_button_id = 101, exit_button_id = 102, game_combobox_id = 103;
HBITMAP hBitMap;
HWND main_hwnd;

class games_t {

private:
    std::vector<std::shared_ptr<Game>> game_ptrs;
    int idx = 0;

public:
    static const TCHAR* game_strs[];
    Game* game()
    {
        return this->game_ptrs[this->idx].get();
    }
    void reset_idx(int idx)
    {
        this->idx = idx;
    }
    void add_game(std::shared_ptr<Game> game)
    {
        this->game_ptrs.push_back(game);
    }
};
games_t games;

const TCHAR* games_t::game_strs[] = {
    TEXT("推箱子"), TEXT("俄罗斯方块")
};

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam);
int init_wnd_cls(WNDCLASSEX& wndClass, HINSTANCE& hInstance)
{
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon
        = (HICON)::LoadImage(NULL, TEXT("./resource/icon.ico"), IMAGE_ICON, 0,
            0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = WIN_CLS;

    if (!RegisterClassEx(&wndClass))
        return ERR;
    return OK;
}

void init_game()
{
    std::shared_ptr<Game> sokoBan_ptr { new SokoBanGame() };
    std::shared_ptr<Game> tetris_ptr { new Tetris() };
    games.add_game(sokoBan_ptr);
    games.add_game(tetris_ptr);
}

int WINAPI
WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    WNDCLASSEX wndClass = { 0 };
    init_game();
    init_wnd_cls(wndClass, hInstance);

    ::screen_width = GetSystemMetrics(SM_CXSCREEN);
    ::screen_height = GetSystemMetrics(SM_CYSCREEN);

    main_hwnd
        = CreateWindow(WIN_CLS, WINDOW_TITLE, WS_POPUP, 0, 0, ::screen_width,
            ::screen_height, NULL, NULL, hInstance, NULL);
    ShowWindow(main_hwnd, nShowCmd);
    UpdateWindow(main_hwnd);

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            if (games.game()->is_game_init()) {
                games.game()->game_loop();
            }
        }
    }

    UnregisterClass(WIN_CLS, wndClass.hInstance);
    return 0;
}

void process_keydown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (games.game()->is_game_init()) {
        games.game()->game_process_key_down(hwnd, message, wParam, lParam);
    } else {
        switch (wParam) {
        case VK_ESCAPE:
            DestroyWindow(hwnd);
            PostQuitMessage(0);
            break;
        }
    }
}

void paint_window(HWND main_hwnd)
{
    HDC hdc = GetDC(main_hwnd);

    HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    RECT rect { 0, 0, screen_width, screen_height };
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);

    ReleaseDC(main_hwnd, hdc);
}
void init_game_combobox()
{
    const TCHAR** game_strs = games_t::game_strs;
    int games_len = LEN(games_t::game_strs);
    for (int idx = 0; idx < games_len; idx++) {
        SendMessage(game_combobox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)game_strs[idx]);
    }
    SendMessage(game_combobox, CB_SETCURSEL, 0, 0);
}

LRESULT CALLBACK
WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MAIN_WINDOW:
    case WM_CREATE: {

        paint_window(hwnd);
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
        main_button = CreateWindow(
            TEXT("BUTTON"), TEXT("开始游戏"),
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            ::screen_width / 2 - 20, ::screen_height / 2 - 50, 100, 40, hwnd,
            (HMENU)main_button_id,
            hInstance, NULL);
        exit_button = CreateWindow(
            TEXT("BUTTON"), TEXT("退出游戏"),
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            ::screen_width / 2 - 20, ::screen_height / 2 + 100, 100, 40, hwnd,
            (HMENU)exit_button_id,
            hInstance, NULL);
        game_combobox = CreateWindow(
            TEXT("COMBOBOX"), TEXT("DROPDOWNLIST"),
            WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_HASSTRINGS | CBS_DROPDOWNLIST,
            ::screen_width / 2 - 20, ::screen_height / 2 - 150, 100, 300, hwnd,
            (HMENU)game_combobox_id,
            hInstance, NULL);
        init_game_combobox();
        break;
    }
    case WM_COMMAND: {

        const int window_id = LOWORD(wParam);
        if (window_id == main_button_id) {
            games.game()->game_init(main_hwnd);
            DestroyWindow(main_button);
            DestroyWindow(exit_button);
            DestroyWindow(game_combobox);
        } else if (window_id == exit_button_id) {
            DestroyWindow(hwnd);
            PostQuitMessage(0);
        } else if (window_id == game_combobox_id) {
            int event_id = HIWORD(wParam);
            if (event_id == CBN_SELCHANGE) {
                int item_index = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                games.reset_idx(item_index);
            }
        }
        break;
    }
    case WM_DRAWITEM:
        break;
    case WM_KEYDOWN:
        process_keydown(hwnd, message, wParam, lParam);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}
