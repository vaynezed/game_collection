#include <tchar.h>
#include "common.hpp"
#include "game.hpp"
#include <windows.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")

const TCHAR* WINDOW_TITLE = TEXT("游戏集合");
const TCHAR* WIN_CLS = TEXT("MainWin");

constexpr int ERR { -1 }, OK { 0 };
HWND main_button, exit_button;
constexpr int main_button_id = 101, exit_button_id = 102;
HBITMAP hBitMap;

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
    wndClass.hIcon = (HICON)::LoadImage(NULL, TEXT("./resource/icon.ico"), IMAGE_ICON, 0, 0,
        LR_DEFAULTSIZE | LR_LOADFROMFILE);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = WIN_CLS;

    if (!RegisterClassEx(
            &wndClass))
        return ERR;
    return OK;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    WNDCLASSEX wndClass = { 0 };
    init_wnd_cls(wndClass, hInstance);

    ::screen_width = GetSystemMetrics(SM_CXSCREEN);
    ::screen_height = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindow(WIN_CLS, WINDOW_TITLE, WS_POPUP,
        0, 0, ::screen_width, ::screen_height, NULL, NULL,
        hInstance, NULL);
    ShowWindow(hwnd, nShowCmd);
    UpdateWindow(hwnd);


    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(
                &msg, 0, 0, 0,
                PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            if (::game_init_flag) {
                game_loop();
            }
        }
    }

    UnregisterClass(WIN_CLS,
        wndClass.hInstance);
    return 0;
}

void process_keydown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (::game_init_flag) {
        game_process_key_down(hwnd, message, wParam, lParam);
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam)
{
    constexpr int MAIN_WINDOW { WM_APP + 1 };
    switch (message) {
    case MAIN_WINDOW:
    case WM_CREATE:
        paint_window(hwnd);
        main_button = CreateWindow(TEXT("BUTTON"), TEXT("开始游戏"),
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            ::screen_width / 2 - 20, ::screen_height / 2 - 50, 100, 40,
            hwnd, (HMENU)main_button_id,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        exit_button = CreateWindow(TEXT("BUTTON"), TEXT("退出游戏"),
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            ::screen_width / 2 - 20, ::screen_height / 2 + 100, 100,
            40, hwnd, (HMENU)exit_button_id,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == main_button_id) {
            game_init(hwnd);
            DestroyWindow(main_button);
            DestroyWindow(exit_button);
        } else if (LOWORD(wParam) == exit_button_id) {
            DestroyWindow(hwnd);
            PostQuitMessage(0);
        }
        break;
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
