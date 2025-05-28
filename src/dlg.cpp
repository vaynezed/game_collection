#include "dlg.hpp"
#include "../resource.h"
#include "WinUser.h"
#include <cctype>

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
default_dlg_wnd_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG: {
        HWND parent_hwnd = GetParent(hDlg);
        EnableWindow(parent_hwnd, FALSE);
        CenterDialog(hDlg);
        return TRUE;
    }

    case WM_COMMAND: {
        INT32 btn_id = LOWORD(wParam);
        EndDialog(hDlg, btn_id);
        return TRUE;
        break;
    }
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

template <typename T>
inline INT_PTR Dialog<T>::dlg_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return INT_PTR();
}
inline INT_PTR Dialog<struct user_info_t>::dlg_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static Dialog<struct user_info_t>* pThis = nullptr;
    switch (message) {
    case WM_INITDIALOG: {
        pThis = reinterpret_cast<Dialog<struct user_info_t>*>(lParam);
        HWND parent_hwnd = GetParent(hDlg);
        EnableWindow(parent_hwnd, FALSE);
        CenterDialog(hDlg);
        return TRUE;
    }
    case WM_COMMAND: {
        INT32 btn_id = LOWORD(wParam);
        if (btn_id == IDYES || btn_id == IDCANCEL) {
            EndDialog(hDlg, btn_id);
        }
        return TRUE;
        break;
    }
    case WM_DESTROY: {
        HWND parent_hwnd = GetParent(hDlg);
        EnableWindow(parent_hwnd, TRUE);
        TCHAR buf[256];
        GetDlgItemText(hDlg, IDE_ROOM_ID, buf, sizeof(buf) / sizeof(buf[0]));
        pThis->data.room_id = std::wstring(buf);
        break;
    }

    default:
        return FALSE;
    }
    return FALSE;
}

int LoginDialog::show(HWND* hwnd)
{
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(*hwnd, GWLP_HINSTANCE);
    int ret = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CONNECT_DLG),
        *hwnd, Dialog<struct user_info_t>::dlg_proc, (LPARAM)this);
    return TRUE;
}