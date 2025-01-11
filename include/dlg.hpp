#pragma once
#include "../resource.h"
#include <Windows.h>

extern INT_PTR CALLBACK default_dlg_wnd_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern void CenterDialog(HWND hwndDlg);
