#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 引入自定义头文件（直接使用 globals.h 中的 extern 声明，不再在本源文件中重复定义全局变量，彻底解决 LNK2005）
#include "globals.h"
#include "worker.h"
#include "action.h"

// 全局 UI 控件句柄外部引用声明（实体已在 main.c 中定义）
extern HWND hEditMin;
extern HWND hEditMax;
extern HWND hCmbBtnType;
extern HWND hCmbActType;
extern HWND hCmbHkToggle;
extern HWND hCmbHkStop;
extern HWND hCmbHkBind;
extern HWND hBtnApply;
extern HWND hStatusLabel;
extern HWND hBindLabel;

void start_recording() {
    if (is_recording) return;
    is_recording = true;
    current_points = 0;
    SetWindowTextA(hStatusLabel, ">> 状态: 录制中... 按 P 暂停，C 取消，R 重新录制");
    MessageBox(NULL, "鼠标轨迹录制已开始！\n移动鼠标即可录制路径（鼠标需悬停在窗口上）。\n按 P 暂停/继续，按 C 取消，按 R 重新开始。", "鼠标轨迹录制", MB_OK | MB_ICONINFORMATION);
}

void stop_recording() {
    if (!is_recording) return;
    is_recording = false;
    SetWindowTextA(hStatusLabel, ">> 状态: 录制已停止（轨迹已保存）");
}

void replay_trajectory() {
    if (!is_recording || current_points == 0) {
        MessageBox(NULL, "请先录制一条轨迹！", "提示", MB_OK | MB_ICONWARNING);
        return;
    }
    is_replaying = true;
    replay_index = 0;
    replay_start_time = GetTickCount64();
    SetWindowTextA(hStatusLabel, ">> 状态: 回放中... （输入回放次数）");
    char input[10];
    GetWindowTextA(GetDlgItem(NULL, ID_EDIT_MAX), input, sizeof(input));
    replay_repeats = atoi(input);
    if (replay_repeats <= 0) replay_repeats = 1;
    SetWindowTextA(hStatusLabel, ">> 状态: 回放中...");
    MessageBox(NULL, "回放已启动！按 B 停止回放。", "回放提示", MB_OK | MB_ICONINFORMATION);
}

void stop_replay() {
    is_replaying = false;
    SetWindowTextA(hStatusLabel, ">> 状态: 回放已停止");
}

DWORD WINAPI WorkerThread(LPVOID lpParam) {
    HWND hMainWnd = (HWND)lpParam;
    srand((unsigned int)time(NULL));

    while (1) {
        // 原热键侦测逻辑（完整保留）
        if (GetAsyncKeyState(hotkey_bind) & 0x8000) {
            Sleep(300);
        }
        if (GetAsyncKeyState(hotkey_stop) & 0x8000) {
            Sleep(300);
        }
        if (GetAsyncKeyState(hotkey_toggle) & 0x8000) {
            Sleep(300);
        }

        // 执行动作（原逻辑）
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

        // ====================== 新增：轨迹录制与回放（精确毫秒级） ======================
        if (is_recording) {
            POINT pt;
            if (GetCursorPos(&pt)) {
                if (current_points < MAX_POINTS) {
                    points[current_points].x = pt.x;
                    points[current_points].y = pt.y;
                    points[current_points].timestamp = GetTickCount64() - replay_start_time;
                    current_points++;
                }
            }
        }

        if (is_replaying && current_points > 0) {
            if (replay_index < current_points) {
                unsigned long long elapsed = GetTickCount64() - replay_start_time;
                while (GetTickCount64() - replay_start_time < elapsed) {
                    Sleep(1);
                }
                INPUT input = {0};
                input.type = INPUT_MOUSE;
                input.mi.dx = points[replay_index].x;
                input.mi.dy = points[replay_index].y;
                input.mi.dwFlags = MOUSEEVENTF_MOVE;
                SendInput(1, &input, sizeof(INPUT));
                replay_index++;
            } else {
                if (replay_repeats > 1) {
                    replay_repeats--;
                    replay_index = 0;
                } else {
                    stop_replay();
                }
            }
        }
    }
    return 0;
}
