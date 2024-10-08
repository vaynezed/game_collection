#include "dlg.hpp"
#include <cctype>

void CenterDialog(HWND hwndDlg)
{
    RECT rcDlg;
    RECT rcScreen;
    int dx, dy;

    // ��ȡ�Ի���ĳߴ�
    GetWindowRect(hwndDlg, &rcDlg);
    dx = rcDlg.right - rcDlg.left;
    dy = rcDlg.bottom - rcDlg.top;

    // ��ȡ��Ļ�ĳߴ�
    rcScreen.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    rcScreen.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    rcScreen.right = rcScreen.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    rcScreen.bottom = rcScreen.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // ����Ի�����е�λ��
    int xPos = (rcScreen.right - dx) / 2;
    int yPos = (rcScreen.bottom - dy) / 2;

    // ���öԻ����λ��
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