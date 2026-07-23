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

    // ====== 新增：录制状态辅助变量 ======
    short last_lbtn = 0;
    short last_rbtn = 0;
    DWORD last_time = 0;

    while (1) {
        // ====== 原有：侦测“绑定/解绑”热键 ======
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

        // 原有：侦测“停止”热键
        if (GetAsyncKeyState(hotkey_stop) & 0x8000) {
            if (is_active) {
                is_active = false;
                SetWindowTextA(hStatusLabel, ">> 状态: 已强制停止");
            } else {
                SetWindowTextA(hStatusLabel, ">> 状态: 已强制停止");
            }
            Sleep(300); 
        }

        // 原有：侦测“开启/暂停”热键
        if (GetAsyncKeyState(hotkey_toggle) & 0x8000) {
            is_active = !is_active;
            if (is_active) {
                SetWindowTextA(hStatusLabel, ">> 状态: 运行中...");
            } else {
                SetWindowTextA(hStatusLabel, ">> 状态: 已暂停");
            }
            Sleep(300); 
        }

        // ====== 新增：侦测“录制”热键 ======
        if (GetAsyncKeyState(hotkey_record) & 0x8000) {
            is_recording = !is_recording;
            if (is_recording) {
                record_count = 0; // 开始录制时清空旧数据
                // 记录开启录制瞬间的按键状态，防止误触发
                last_lbtn = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
                last_rbtn = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
                last_time = GetTickCount(); 
                SetWindowTextA(hStatusLabel, ">> 状态: 正在录制中...");
            } else {
                SetWindowTextA(hStatusLabel, ">> 状态: 录制完成");
            }
            Sleep(300); // 防抖动
        }

        // ====== 新增：侦测“回放”热键 ======
        if (GetAsyncKeyState(hotkey_playback) & 0x8000) {
            if (!is_recording && record_count > 0) {
                SetWindowTextA(hStatusLabel, ">> 状态: 正在回放中...");
                execute_playback(); // 调用独立的回放函数
                SetWindowTextA(hStatusLabel, ">> 状态: 已准备就绪");
            }
            Sleep(300); // 防抖动
        }

        // ====== 新增：数据采集逻辑 ======
        if (is_recording && record_count < MAX_RECORD_SIZE) {
            short curr_lbtn = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
            short curr_rbtn = GetAsyncKeyState(VK_RBUTTON) & 0x8000;
            DWORD curr_time = GetTickCount();
            
            bool action_detected = false;
            UINT msg_type = 0;

            // 状态机：边缘检测（按键从松开变为按下，或从按下变为松开）
            if (curr_lbtn && !last_lbtn) { msg_type = WM_LBUTTONDOWN; action_detected = true; }
            else if (!curr_lbtn && last_lbtn) { msg_type = WM_LBUTTONUP; action_detected = true; }
            else if (curr_rbtn && !last_rbtn) { msg_type = WM_RBUTTONDOWN; action_detected = true; }
            else if (!curr_rbtn && last_rbtn) { msg_type = WM_RBUTTONUP; action_detected = true; }

            if (action_detected) {
                POINT pt;
                GetCursorPos(&pt);
                
                record_buffer[record_count].pt = pt;
                record_buffer[record_count].msg_type = msg_type;
                
                // 计算动作间隔，首个动作延迟为 0
                if (record_count == 0) {
                    record_buffer[record_count].delay = 0;
                } else {
                    record_buffer[record_count].delay = curr_time - last_time;
                }
                
                last_time = curr_time;
                record_count++;
            }

            // 更新状态
            last_lbtn = curr_lbtn;
            last_rbtn = curr_rbtn;
        }

        // 原有：执行动作与波动区间逻辑
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
