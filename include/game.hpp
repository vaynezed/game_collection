#include <Windows.h>

extern BOOL game_init(HWND hwnd); //在此函数中进行资源的初始化
extern VOID game_update(HWND hwnd,
    ULONGLONG* pre_time); //在此函数中进行绘图代码的书写
extern VOID game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam);
extern BOOL game_cleanup(); //在此函数中进行资源的清理
extern bool game_init_flag;
