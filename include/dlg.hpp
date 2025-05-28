#pragma once
#include "../resource.h"
#include <Windows.h>

extern INT_PTR CALLBACK default_dlg_wnd_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern void CenterDialog(HWND hwndDlg);

#pragma once
#include <Windows.h>
#include <map>
#include <string>

struct user_info_t {
    std::wstring room_id;
};

template <typename T>
class Dialog {
private:
    virtual int show(HWND* hwnd) = 0;
    T data;

public:
    static INT_PTR dlg_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    virtual T get_data()
    {
        return data;
    }
};

class LoginDialog : public Dialog<struct user_info_t> {
public:
    int show(HWND* hwnd) override;
};
