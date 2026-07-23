#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "worker.h"
#include "globals.h"
#include "action.h"

DWORD WINAPI WorkerThread(LPVOID lpParam) {
    HWND hMainWnd = (HWND)lpParam;
    srand((unsigned int)time(NULL));

    short last_lbtn = 0, last_rbtn = 0;
    DWORD last_time = 0;

    while (1) {
        // 绑定热键（保留轻量轮询）
        if (GetAsyncKeyState(hotkey_bind) & 0x8000) {
            if (target_hwnd == NULL) {
                POINT pt;
                GetCursorPos(&pt);
                HWND hwndUnder = WindowFromPoint(pt);
                if (hwndUnder && hwndUnder != hMainWnd) {
                    ScreenToClient(hwndUnder, &pt);
                    target_hwnd = hwndUnder;
                    bind_pt = pt;
                    char title[128] = {0};
                    GetWindowTextA(target_hwnd, title, sizeof(title));
                    if (!title[0]) sprintf(title, "句柄 %p", target_hwnd);
                    char msg[256];
                    sprintf(msg, "已锁定: %s (X:%ld, Y:%ld)", title, pt.x, pt.y);
                    SetWindowTextA(hBindLabel, msg);
                }
            } else {
                target_hwnd = NULL;
                SetWindowTextA(hBindLabel, "未绑定 (全局模式)");
            }
            Sleep(250);
        }

        // 鼠标动作录制采集
        if (is_recording && record_count < MAX_RECORD_SIZE) {
            short cl = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
            short cr = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
            DWORD ct = GetTickCount();

            UINT mtype = 0;
            if (cl && !last_lbtn) mtype = WM_LBUTTONDOWN;
            else if (!cl && last_lbtn) mtype = WM_LBUTTONUP;
            else if (cr && !last_rbtn) mtype = WM_RBUTTONDOWN;
            else if (!cr && last_rbtn) mtype = WM_RBUTTONUP;

            if (mtype) {
                POINT pt;
                GetCursorPos(&pt);
                record_buffer[record_count].pt = pt;
                record_buffer[record_count].msg_type = mtype;
                record_buffer[record_count].delay = (record_count == 0 ? 0 : ct - last_time);
                last_time = ct;
                record_count++;
            }
            last_lbtn = cl;
            last_rbtn = cr;
        }

        // 原有自动点击
        if (is_active) {
            execute_action();
            int iv = interval_min;
            if (interval_max > interval_min)
                iv += rand() % (interval_max - interval_min + 1);
            Sleep(iv);
        } else {
            Sleep(10);
        }
    }
    return 0;
}
