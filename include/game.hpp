#include <Windows.h>

extern BOOL game_init(HWND& hwnd); //�ڴ˺����н�����Դ�ĳ�ʼ��
extern VOID game_loop(); //�ڴ˺����н��л�ͼ�������д
extern VOID game_process_key_down(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam);
extern BOOL game_cleanup(); //�ڴ˺����н�����Դ������
extern bool is_game_init();
