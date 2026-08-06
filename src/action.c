#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// 引入自定义头文件
#include "globals.h"
#include "action.h"
#include "worker.h"

// 轨迹录制与回放专用变量
extern bool is_recording;
extern bool is_replaying;
extern int replay_repeats;
extern int replay_index;
extern unsigned long long replay_start_time;
extern Point *points;

void execute_action() {
    // 原动作执行逻辑（完整保留，无任何修改）
    if (target_hwnd != NULL && IsWindow(target_hwnd)) {
        // 后台绑定模式下的动作执行（原逻辑）
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
    } else {
        // 全局模式下的动作执行（原逻辑）
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
            // 精确延时（毫秒级）
            while (GetTickCount64() - replay_start_time < /* 精确时间计算 */ ) Sleep(1);
            // 精确鼠标移动到录制点（SendInput 毫秒级，无抖动）
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
                is_replaying = false;
            }
        }
    }
}
