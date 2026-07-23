#include <windows.h>
#include "action.h"
#include "globals.h"

void execute_action() {
    int loops = (action_mode == 0) ? 1 : 2;
    UINT down = (action_button == 0) ? WM_LBUTTONDOWN : WM_RBUTTONDOWN;
    UINT up   = (action_button == 0) ? WM_LBUTTONUP   : WM_RBUTTONUP;
    WPARAM wp = (action_button == 0) ? MK_LBUTTON     : MK_RBUTTON;
    LPARAM lp = MAKELPARAM(bind_pt.x, bind_pt.y);

    if (target_hwnd && IsWindow(target_hwnd)) {
        // 后台绑定模式
        for (int i = 0; i < loops; i++) {
            PostMessage(target_hwnd, down, wp, lp);
            PostMessage(target_hwnd, up, 0, lp);
            if (loops > 1 && i == 0) Sleep(30);
        }
    } else {
        // 全局前台模式
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        DWORD dflag = (action_button == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
        DWORD uflag = (action_button == 0) ? MOUSEEVENTF_LEFTUP   : MOUSEEVENTF_RIGHTUP;
        for (int i = 0; i < loops; i++) {
            input.mi.dwFlags = dflag; SendInput(1, &input, sizeof(INPUT));
            input.mi.dwFlags = uflag; SendInput(1, &input, sizeof(INPUT));
            if (loops > 1 && i == 0) Sleep(30);
        }
    }
}

void execute_playback() {
    for (int p = 0; p < playback_count; p++) {
        for (int i = 0; i < record_count; i++) {
            if (GetAsyncKeyState(hotkey_stop) & 0x8000) {
                is_active = false;
                return;
            }
            if (record_buffer[i].delay > 0) Sleep(record_buffer[i].delay);

            UINT msg = record_buffer[i].msg_type;
            if (target_hwnd && IsWindow(target_hwnd)) {
                // 后台模式
                POINT pt = record_buffer[i].pt;
                ScreenToClient(target_hwnd, &pt);
                LPARAM lp = MAKELPARAM(pt.x, pt.y);
                WPARAM wp = 0;
                if (msg == WM_LBUTTONDOWN) wp = MK_LBUTTON;
                else if (msg == WM_RBUTTONDOWN) wp = MK_RBUTTON;
                PostMessage(target_hwnd, msg, wp, lp);
            } else {
                // 前台模式
                INPUT input = {0};
                input.type = INPUT_MOUSE;
                if (msg == WM_LBUTTONDOWN) input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                else if (msg == WM_LBUTTONUP) input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                else if (msg == WM_RBUTTONDOWN) input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                else if (msg == WM_RBUTTONUP) input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                SendInput(1, &input, sizeof(INPUT));
            }
        }
    }
}
