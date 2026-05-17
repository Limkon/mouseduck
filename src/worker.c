#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include "worker.h"
#include "globals.h"
#include "action.h"

DWORD WINAPI WorkerThread(LPVOID lpParam) {
    // lpParam 传入的是主窗口的句柄 HWND
    HWND hMainWnd = (HWND)lpParam;
    
    // 初始化随机数种子
    srand((unsigned int)time(NULL));

    while (1) {
        // ====== 修改：侦测 F9 停止热键 (仅停止任务，不退出 GUI) ======
        if (GetAsyncKeyState(VK_F9) & 0x8000) {
            is_active = false; // 强制设置为停止状态
            SetWindowTextA(hStatusLabel, ">> 状态: 已停止 [F8开启/F9停止]");
            Sleep(300); // 防抖动延迟
        }
        // ==============================================================

        // 侦测 F8 状态切换热键
        if (GetAsyncKeyState(VK_F8) & 0x8000) {
            is_active = !is_active;
            if (is_active) {
                SetWindowTextA(hStatusLabel, ">> 状态: 运行中 [F8暂停/F9停止]");
            } else {
                SetWindowTextA(hStatusLabel, ">> 状态: 已暂停 [F8开启/F9停止]");
            }
            Sleep(300); // 防抖动延迟
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
