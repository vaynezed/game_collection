#pragma once
#include "../resource.h"
#include <Windows.h>
extern INT_PTR CALLBACK next_level_dlg_wnd_proc(HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam);

extern INT_PTR CALLBACK pre_level_dlg_wnd_proc(HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK restart_dlg_wnd_proc(HWND hDlg, UINT message,
    WPARAM wParam, LPARAM lParam);
