#include "dlg.hpp"

void CenterDialog(HWND hwndDlg)
{
    RECT rcDlg;
    RECT rcScreen;
    int dx, dy;

    // 获取对话框的尺寸
    GetWindowRect(hwndDlg, &rcDlg);
    dx = rcDlg.right - rcDlg.left;
    dy = rcDlg.bottom - rcDlg.top;

    // 获取屏幕的尺寸
    rcScreen.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    rcScreen.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    rcScreen.right = rcScreen.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    rcScreen.bottom = rcScreen.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // 计算对话框居中的位置
    int xPos = (rcScreen.right - dx) / 2;
    int yPos = (rcScreen.bottom - dy) / 2;

    // 设置对话框的位置
    SetWindowPos(hwndDlg, NULL, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

INT_PTR CALLBACK
next_level_dlg_wnd_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG: {
        HWND parent_hwnd = GetParent(hDlg);
        EnableWindow(parent_hwnd, FALSE);
        CenterDialog(hDlg);
        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_NEXT_LEVEL) {
            EndDialog(hDlg, ID_NEXT_LEVEL);
            return TRUE;
        } else if (LOWORD(wParam) == ID_GOBACK) {
            EndDialog(hDlg, ID_GOBACK);
            return TRUE;
        }
        break;
    case WM_DESTROY: {
        HWND parent_hwnd = GetParent(hDlg);
        EnableWindow(parent_hwnd, TRUE);
        break;
    }

    default:
        return FALSE;
    }
    return FALSE;
}

INT_PTR CALLBACK
pre_level_dlg_wnd_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        CenterDialog(hDlg);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_PRE_LEVEL) {
            EndDialog(hDlg, ID_PRE_LEVEL);
            return TRUE;
        } else if (LOWORD(wParam) == ID_GOBACK) {
            EndDialog(hDlg, ID_GOBACK);
            return TRUE;
        }
        break;

    default:
        return FALSE;
    }
    return FALSE;
}

INT_PTR CALLBACK
restart_dlg_wnd_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        CenterDialog(hDlg);
        return TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_RESTART_BTN) {
            EndDialog(hDlg, ID_RESTART_BTN);
            return TRUE;
        } else if (LOWORD(wParam) == ID_GOBACK) {
            EndDialog(hDlg, ID_GOBACK);
            return TRUE;
        }
        break;

    default:
        return FALSE;
    }
    return FALSE;
}