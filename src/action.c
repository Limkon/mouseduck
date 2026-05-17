#include <windows.h>
#include "action.h"
#include "globals.h"

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
