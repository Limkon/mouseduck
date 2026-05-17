#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include "worker.h"
#include "globals.h"
#include "action.h"

DWORD WINAPI WorkerThread(LPVOID lpParam) {
    // lpParam 传入的是主窗口的句柄 HWND，用于发送退出消息
    HWND hMainWnd = (HWND)lpParam;
    
    // 初始化随机数种子
    srand((unsigned int)time(NULL));

    while (1) {
        // 侦测 F9 退出热键
        if (GetAsyncKeyState(VK_F9) & 0x8000) {
            // 向主窗口发送关闭消息，实现优雅退出
            PostMessage(hMainWnd, WM_CLOSE, 0, 0); 
            break;
        }

        // 侦测 F8 状态切换热键
        if (GetAsyncKeyState(VK_F8) & 0x8000) {
            is_active = !is_active;
            if (is_active) {
                SetWindowTextA(hStatusLabel, ">> 状态: 运行中 [F8暂停/F9退出]");
            } else {
                SetWindowTextA(hStatusLabel, ">> 状态: 已暂停 [F8开启/F9退出]");
            }
            Sleep(300); // 防抖动
        }

        // 执行与波动逻辑
        if (is_active) {
            execute_action();
            
            int current_interval = interval_min;
            if (interval_max > interval_min) {
                // 产生区间内的随机波动
                current_interval = interval_min + rand() % (interval_max - interval_min + 1);
            }
            Sleep(current_interval);
        } else {
            Sleep(10); // 暂停时释放 CPU
        }
    }
    return 0;
}
