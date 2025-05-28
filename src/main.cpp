
#include "common.hpp"
#include "games/games.hpp"
#define LEN(A) (sizeof(A) / sizeof(A[0]))

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "ws2_32.lib") // 链接 Winsock 库

const TCHAR* WINDOW_TITLE = TEXT("游戏集合");
const TCHAR* WIN_CLS = TEXT("MainWin");

constexpr int ERR { -1 }, OK { 0 };
HWND main_button, exit_button, game_combobox, game_model_combobox;
constexpr int main_button_id = 101, exit_button_id = 102, game_combobox_id = 103, game_model_combobox_id = 104;
HBITMAP hBitMap;
HWND main_hwnd;
void reset_game_models(const std::vector<std::wstring>& game_models);

class games_t {

private:
    std::vector<std::shared_ptr<Game>> game_ptrs;
    int game_model_idx = 0;
    int idx = 0;

public:
    std::vector<std::wstring> games_str;
    Game* game()
    {
        return this->game_ptrs[this->idx].get();
    }
    void reset_idx(int idx)
    {
        size_t game_size = game_ptrs.size();
        if (idx < 0 || idx > game_size) {
            std::string msg = format("Index out of range %d", idx, 25);
            throw std::out_of_range(msg);
        }
        this->idx = idx;
        std::vector<std::wstring> game_models = std::move(this->game()->game_models());
        reset_game_models(game_models);
    }
    void reset_game_model_index(int idx)
    {
        this->game_model_idx = idx;
        this->game()->set_game_model(idx);
    }
    void add_game(std::shared_ptr<Game> game)
    {
        this->game_ptrs.push_back(game);
        this->games_str.push_back(game->to_string());
    }
};
games_t games;

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
    std::shared_ptr<Game> snake_ptr { new Snake() };
    std::shared_ptr<Game> g_2048_ptr { new G_2048_t() };
    std::shared_ptr<Game> chess_ptr { new Chess() };
    games.add_game(sokoBan_ptr);
    games.add_game(tetris_ptr);
    games.add_game(snake_ptr);
    games.add_game(g_2048_ptr);
    games.add_game(chess_ptr);
}

extern FILE* log_file;
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
    RAII<FILE*> log_file_magment(fopen("log.txt", "w"), fclose);
    log_file = log_file_magment.get();

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
    const std::vector<std::wstring>& games_str = games.games_str;
    size_t size = games_str.size();
    for (int idx = 0; idx < size; idx++) {
        SendMessage(game_combobox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)(games_str[idx].c_str()));
    }
    SendMessage(game_combobox, CB_SETCURSEL, 0, 0);
}
void reset_game_models(const std::vector<std::wstring>& game_models)
{
    SendMessage(game_model_combobox, CB_RESETCONTENT, 0, 0);
    for (const auto& model : game_models) {
        SendMessage(game_model_combobox, CB_ADDSTRING, 0, (LPARAM)model.c_str());
    }
}

void init_buttons(const HWND& hwnd)
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    main_button = (HWND)CreateWindow(
        TEXT("BUTTON"), TEXT("开始游戏"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        ::screen_width / 2 - 20, ::screen_height / 2 - 50, 100, 40, hwnd,
        (HMENU)main_button_id,
        hInstance, NULL);
    exit_button = (HWND)CreateWindow(
        TEXT("BUTTON"), TEXT("退出游戏"),
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        ::screen_width / 2 - 20, ::screen_height / 2 + 100, 100, 40, hwnd,
        (HMENU)exit_button_id,
        hInstance, NULL);
    game_combobox = (HWND)CreateWindow(
        TEXT("COMBOBOX"), TEXT("DROPDOWNLIST"),
        WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_HASSTRINGS | CBS_DROPDOWNLIST,
        ::screen_width / 2 - 20, ::screen_height / 2 - 200, 100, 300, hwnd,
        (HMENU)game_combobox_id,
        hInstance, NULL);

    game_model_combobox = (HWND)CreateWindow(
        TEXT("COMBOBOX"), TEXT("DROPDOWNLIST"),
        WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_HASSTRINGS | CBS_DROPDOWNLIST,
        ::screen_width / 2 - 20, ::screen_height / 2 - 150, 100, 300, hwnd,
        (HMENU)game_model_combobox_id,
        hInstance, NULL);
}

LRESULT CALLBACK
WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case MAIN_WINDOW:
    case WM_CREATE: {
        init_buttons(hwnd);
        paint_window(hwnd);
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
            DestroyWindow(game_model_combobox);
        } else if (window_id == exit_button_id) {
            DestroyWindow(hwnd);
            PostQuitMessage(0);
        } else if (window_id == game_combobox_id) {
            int event_id = HIWORD(wParam);
            if (event_id == CBN_SELCHANGE) {
                int item_index = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                games.reset_idx(item_index);
            }
        } else if (window_id == game_model_combobox_id) {
            int event_id = HIWORD(wParam);
            if (event_id == CBN_SELCHANGE) {
                int item_index = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                games.reset_game_model_index(item_index);
            }
        }
        break;
    }
    case WM_DRAWITEM:
        break;
    case WM_KEYDOWN:
    case WM_LBUTTONDOWN:
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
