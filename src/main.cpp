#include <tchar.h>
#include "game.hpp"
#include <windows.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Msimg32.lib")

//-----------------------------------【宏定义部分】--------------------------------------------
//	描述：定义一些辅助宏
//------------------------------------------------------------------------------------------------
#define WINDOW_TITLE \
    L"【致我们永不熄灭的游戏开发梦想】Windows消息处理之 键盘消息处理 " //为窗口标题定义的宏

//-----------------------------------【全局函数声明部分】-------------------------------------
//	描述：全局函数声明，防止“未声明的标识”系列错误
//------------------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam); //窗口过程函数
constexpr int ERR = -1;
constexpr int OK = 0;
int screen_height { -1 }, screen_width { -1 };
HWND main_button, exit_button;
constexpr int main_button_id = 101, exit_button_id = 102;
HBITMAP hBitMap;
int init_wnd_cls(WNDCLASSEX& wndClass, HINSTANCE& hInstance)
{
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = (HICON)::LoadImage(NULL, L"icon.ico", IMAGE_ICON, 0, 0,
        LR_DEFAULTSIZE | LR_LOADFROMFILE);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = L"ForTheDreamOfGameDevelop";

    if (!RegisterClassEx(
            &wndClass)) //设计完窗口后，需要对窗口类进行注册，这样才能创建该类型的窗口
        return ERR;
    return OK;
}

//-----------------------------------【WinMain(
//)函数】--------------------------------------
//	描述：Windows应用程序的入口函数，我们的程序从这里开始
//------------------------------------------------------------------------------------------------
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    WNDCLASSEX wndClass = { 0 };
    init_wnd_cls(wndClass, hInstance);

    ::screen_width = GetSystemMetrics(SM_CXSCREEN);
    ::screen_height = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindow(L"ForTheDreamOfGameDevelop", WINDOW_TITLE, WS_POPUP,
        0, 0, ::screen_width, ::screen_height, NULL, NULL,
        hInstance, NULL);
    ShowWindow(hwnd, nShowCmd);
    UpdateWindow(hwnd);

    ULONGLONG
    g_tPre = 0,
    g_tNow = 0; //声明l两个函数来记录时间,g_tPre记录上一次绘图的时间，g_tNow记录此次准备绘图的时间
    //【5】消息循环过程
    MSG msg = { 0 }; //定义并初始化msg
    while (msg.message != WM_QUIT) //使用while循环，如果消息不是WM_QUIT消息，就继续循环
    {
        if (PeekMessage(
                &msg, 0, 0, 0,
                PM_REMOVE)) //查看应用程序消息队列，有消息时将队列中的消息派发出去。
        {
            TranslateMessage(&msg); //将虚拟键消息转换为字符消息
            DispatchMessage(&msg); //分发一个消息给窗口程序。
        } else {
            if (::game_init_flag) {
                g_tNow = GetTickCount64(); //获取当前系统时间
                if (g_tNow - g_tPre >= 50) //当此次循环运行与上次绘图时间相差0.05秒时再进行重绘操作
                    game_paint(hwnd, &g_tPre);
            }
        }
    }

    //【6】窗口类的注销
    UnregisterClass(L"ForTheDreamOfGameDevelop",
        wndClass.hInstance); //程序准备结束，注销窗口类
    return 0;
}

void process_keydown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (::game_init_flag) {
        game_process_key_down(hwnd, message, wParam, lParam);
    } else {
        switch (wParam) {
        case VK_ESCAPE: //按下【Esc】键
            DestroyWindow(hwnd); // 销毁窗口, 并发送一条WM_DESTROY消息
            PostQuitMessage(0); //结束程序
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
//-----------------------------------【WndProc(
//)函数】--------------------------------------
//	描述：窗口过程函数WndProc,对窗口消息进行处理
//------------------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam)
{
    constexpr int MAIN_WINDOW { WM_APP + 1 };
    switch (message) // switch语句开始
    {
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
        hBitMap = (HBITMAP)LoadImage(NULL, TEXT("male.bmp"), IMAGE_BITMAP, 0, 0,
            LR_LOADFROMFILE);
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
    case WM_DESTROY: //若是窗口销毁消息
        PostQuitMessage(0); //向系统表明有个线程有终止请求。用来响应WM_DESTROY消息
        break; //跳出该switch语句

    default: //若上述case条件都不符合，则执行该default语句
        return DefWindowProc(hwnd, message, wParam, lParam); //调用缺省的窗口过程
    }

    return 0; //正常退出
}
