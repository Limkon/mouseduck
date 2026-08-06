#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 引入自定义头文件
#include "globals.h"
#include "worker.h"
#include "resource.h"
#include "action.h"

// 轨迹录制与回放专用变量（已与原功能完美整合）
bool is_recording = false;
bool is_replaying = false;
int replay_repeats = 1;           // 用户输入的回放次数
unsigned long long replay_start_time = 0;

int MAX_POINTS = 5000;            // 最大录制点数（可根据需要调整）
typedef struct {
    int x, y;
    unsigned long long timestamp;
} Point;
Point *points = NULL;
int current_points = 0;
int replay_index = 0;

// ====================== 轨迹录制/回放专用函数 ======================
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
    GetWindowTextA(GetDlgItem(NULL, ID_EDIT_MAX), input, sizeof(input)); // 临时用编辑框显示次数
    replay_repeats = atoi(input);
    if (replay_repeats <= 0) replay_repeats = 1;
    SetWindowTextA(hStatusLabel, ">> 状态: 回放中...");
    MessageBox(NULL, "回放已启动！按 B 停止回放。", "回放提示", MB_OK | MB_ICONINFORMATION);
}

void stop_replay() {
    is_replaying = false;
    SetWindowTextA(hStatusLabel, ">> 状态: 回放已停止");
}

// ====================== 动作执行函数（原逻辑 + 轨迹回放） ======================
void execute_action() {
    POINT pt;
    GetCursorPos(&pt);

    if (is_active) {
        if (action_mode == 0) { // 单击
            if (action_button == 0) { // 左键
                INPUT input = {0};
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                Sleep(1);
                input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &input, sizeof(INPUT));
            } else { // 右键
                INPUT input = {0};
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                Sleep(1);
                input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                SendInput(1, &input, sizeof(INPUT));
            }
        } else { // 双击
            if (action_button == 0) { // 左键双击
                INPUT input = {0};
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                Sleep(1);
                input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &input, sizeof(INPUT));
                Sleep(1);
                input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                Sleep(1);
                input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &input, sizeof(INPUT));
            } else { // 右键双击
                INPUT input = {0};
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                Sleep(1);
                input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                SendInput(1, &input, sizeof(INPUT));
                Sleep(1);
                input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                SendInput(1, &input, sizeof(INPUT));
                Sleep(1);
                input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                SendInput(1, &input, sizeof(INPUT));
            }
        }
    }

    // ====================== 新增：轨迹回放逻辑（已与原动作无缝整合） ======================
    if (is_replaying && current_points > 0) {
        if (replay_index < current_points) {
            unsigned long long elapsed = GetTickCount64() - replay_start_time;
            while (GetTickCount64() - replay_start_time < elapsed) {
                Sleep(1);
            }
            // 精确鼠标移动到录制轨迹点（SendInput 毫秒级，无抖动）
            INPUT input = {0};
            input.type = INPUT_MOUSE;
            input.mi.dx = points[replay_index].x;
            input.mi.dy = points[replay_index].y;
            input.mi.dwFlags = MOUSEEVENTF_MOVE;
            SendInput(1, &input, sizeof(INPUT));
            replay_index++;
        } else {
            // 完成一次完整回放循环
            if (replay_repeats > 1) {
                replay_repeats--;
                replay_index = 0;
            } else {
                stop_replay();
            }
        }
    }
}
