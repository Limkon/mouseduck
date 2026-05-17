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
        // 侦测动态配置的“停止”热键
        if (GetAsyncKeyState(hotkey_stop) & 0x8000) {
            // 只有在运行或暂停状态下才更新UI，避免频繁重绘
            if (is_active) {
                is_active = false;
                SetWindowTextA(hStatusLabel, ">> 状态: 已强制停止");
            } else {
                SetWindowTextA(hStatusLabel, ">> 状态: 已强制停止");
            }
            Sleep(300); // 防抖动延迟
        }

        // 侦测动态配置的“开启/暂停”热键
        if (GetAsyncKeyState(hotkey_toggle) & 0x8000) {
            is_active = !is_active;
            if (is_active) {
                SetWindowTextA(hStatusLabel, ">> 状态: 运行中...");
            } else {
                SetWindowTextA(hStatusLabel, ">> 状态: 已暂停");
            }
            Sleep(300); // 防抖动延迟
        }

        // 执行动作与波动区间逻辑
        if (is_active) {
            execute_action();
            
            int current_interval = interval_min;
            if (interval_max > interval_min) {
                // 在最小和最大之间产生随机波动
                current_interval = interval_min + rand() % (interval_max - interval_min + 1);
            }
            Sleep(current_interval);
        } else {
            // 暂停时释放 CPU 资源
            Sleep(10); 
        }
    }
    return 0;
}
