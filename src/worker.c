#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "worker.h"
#include "globals.h"
#include "action.h"

DWORD WINAPI WorkerThread(LPVOID lpParam) {
    // lpParam 传入的是主窗口的句柄 HWND
    HWND hMainWnd = (HWND)lpParam;
    
    // 初始化随机数种子
    srand((unsigned int)time(NULL));

    while (1) {
        // ====== 新增：侦测“绑定/解绑”热键 ======
        if (GetAsyncKeyState(hotkey_bind) & 0x8000) {
            if (target_hwnd == NULL) {
                // 1. 执行绑定逻辑
                POINT pt;
                GetCursorPos(&pt); // 获取当前鼠标的屏幕绝对坐标
                HWND hwndUnderCursor = WindowFromPoint(pt); // 获取坐标处的窗口句柄

                // 确保获取到了句柄，并且没有把自己（本程序窗口）给绑定了
                if (hwndUnderCursor != NULL && hwndUnderCursor != hMainWnd) {
                    // 将屏幕绝对坐标转换为目标窗口内部的相对坐标
                    ScreenToClient(hwndUnderCursor, &pt);
                    
                    target_hwnd = hwndUnderCursor;
                    bind_pt = pt;
                    
                    // 尝试获取窗口标题，如果没有标题则显示句柄地址
                    char title[128];
                    GetWindowTextA(target_hwnd, title, sizeof(title));
                    if (strlen(title) == 0) {
                        sprintf(title, "句柄 %p", target_hwnd);
                    }
                    
                    // 更新 UI 界面上的绑定状态
                    char statusMsg[256];
                    sprintf(statusMsg, "已锁定: %s (X:%ld, Y:%ld)", title, bind_pt.x, bind_pt.y);
                    SetWindowTextA(hBindLabel, statusMsg);
                }
            } else {
                // 2. 执行解绑逻辑
                target_hwnd = NULL;
                SetWindowTextA(hBindLabel, "未绑定 (全局模式)");
            }
            Sleep(300); // 防抖动延迟
        }
        // ========================================

        // 侦测“停止”热键
        if (GetAsyncKeyState(hotkey_stop) & 0x8000) {
            if (is_active) {
                is_active = false;
                SetWindowTextA(hStatusLabel, ">> 状态: 已强制停止");
            } else {
                SetWindowTextA(hStatusLabel, ">> 状态: 已强制停止");
            }
            Sleep(300); 
        }

        // 侦测“开启/暂停”热键
        if (GetAsyncKeyState(hotkey_toggle) & 0x8000) {
            is_active = !is_active;
            if (is_active) {
                SetWindowTextA(hStatusLabel, ">> 状态: 运行中...");
            } else {
                SetWindowTextA(hStatusLabel, ">> 状态: 已暂停");
            }
            Sleep(300); 
        }

        // 执行动作与波动区间逻辑
        if (is_active) {
            execute_action();
            
            int current_interval = interval_min;
            if (interval_max > interval_min) {
                current_interval = interval_min + rand() % (interval_max - interval_min + 1);
            }
            Sleep(current_interval);
        } else {
            Sleep(10); 
        }
    }
    return 0;
}
