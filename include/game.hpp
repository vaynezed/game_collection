#include <Windows.h>

extern BOOL game_init(HWND hwnd); //�ڴ˺����н�����Դ�ĳ�ʼ��
extern VOID game_update(HWND hwnd,
    ULONGLONG* pre_time); //�ڴ˺����н��л�ͼ�������д
extern VOID game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam);
extern BOOL game_cleanup(); //�ڴ˺����н�����Դ������
extern bool game_init_flag;
