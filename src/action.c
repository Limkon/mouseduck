#include <windows.h>
#include "action.h"
#include "globals.h"

// 原有：执行动作函数（完全保留原有功能与逻辑链路）
void execute_action() {
    // 判断执行次数（0:单击则执行1次，1:双击则执行2次）
    int loop_count = (action_mode == 0) ? 1 : 2;

    // 如果目标窗口句柄存在，且该窗口目前仍然有效
    if (target_hwnd != NULL && IsWindow(target_hwnd)) {
        // ==========================================
        // 模式一：后台绑定模式 (使用 PostMessage)
        // ==========================================
        UINT msg_down = (action_button == 0) ? WM_LBUTTONDOWN : WM_RBUTTONDOWN;
        UINT msg_up   = (action_button == 0) ? WM_LBUTTONUP   : WM_RBUTTONUP;
        WPARAM wparam = (action_button == 0) ? MK_LBUTTON     : MK_RBUTTON;
        LPARAM lparam = MAKELPARAM(bind_pt.x, bind_pt.y); // 合成相对坐标

        for (int i = 0; i < loop_count; i++) {
            // 发送按下消息
            PostMessage(target_hwnd, msg_down, wparam, lparam);
            // 发送抬起消息
            PostMessage(target_hwnd, msg_up, 0, lparam);
            
            // 双击系统级延迟
            if (loop_count > 1 && i == 0) {
                Sleep(30); 
            }
        }
    } else {
        // ==========================================
        // 模式二：全局前台模式 (使用 SendInput)
        // ==========================================
        INPUT input = {0};
        input.type = INPUT_MOUSE;

        DWORD down_flag = (action_button == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
        DWORD up_flag   = (action_button == 0) ? MOUSEEVENTF_LEFTUP   : MOUSEEVENTF_RIGHTUP;

        for (int i = 0; i < loop_count; i++) {
            input.mi.dwFlags = down_flag;
            SendInput(1, &input, sizeof(INPUT));

            input.mi.dwFlags = up_flag;
            SendInput(1, &input, sizeof(INPUT));

            // 双击系统级延迟
            if (loop_count > 1 && i == 0) {
                Sleep(30); 
            }
        }
    }
}

// ====== 新增：回放动作函数 ======
void execute_playback() {
    for (int p = 0; p < playback_count; p++) {
        for (int i = 0; i < record_count; i++) {
            // 强制停止热键侦测中断机制 (保护措施)
            if (GetAsyncKeyState(hotkey_stop) & 0x8000) {
                is_active = false;
                return; // 立即终止回放循环
            }

            // 精准还原动作间隔
            if (record_buffer[i].delay > 0) {
                Sleep(record_buffer[i].delay);
            }

            UINT msg = record_buffer[i].msg_type;

            // 完全复用原有的句柄判定逻辑
            if (target_hwnd != NULL && IsWindow(target_hwnd)) {
                // 后台模式：坐标转换与 PostMessage
                POINT pt = record_buffer[i].pt;
                ScreenToClient(target_hwnd, &pt);
                LPARAM lparam = MAKELPARAM(pt.x, pt.y);
                WPARAM wparam = 0; 
                
                // 映射对应的 wParam
                if (msg == WM_LBUTTONDOWN) wparam = MK_LBUTTON;
                else if (msg == WM_RBUTTONDOWN) wparam = MK_RBUTTON;
                
                PostMessage(target_hwnd, msg, wparam, lparam);
            } else {
                // 前台全局模式：SendInput
                INPUT input = {0};
                input.type = INPUT_MOUSE;
                
                // 映射消息到具体的 MOUSEEVENTF 标志位
                if (msg == WM_LBUTTONDOWN) {
                    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                } else if (msg == WM_LBUTTONUP) {
                    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                } else if (msg == WM_RBUTTONDOWN) {
                    input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                } else if (msg == WM_RBUTTONUP) {
                    input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
                } else if (msg == WM_MOUSEMOVE) {
                    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
                    // SendInput 的绝对坐标需要转换到 0~65535 区间
                    input.mi.dx = record_buffer[i].pt.x * 65535 / GetSystemMetrics(SM_CXSCREEN);
                    input.mi.dy = record_buffer[i].pt.y * 65535 / GetSystemMetrics(SM_CYSCREEN);
                }
                
                SendInput(1, &input, sizeof(INPUT));
            }
        }
    }
}
