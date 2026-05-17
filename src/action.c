#include <windows.h>
#include "action.h"
#include "globals.h"

void execute_action() {
    INPUT input = {0};
    input.type = INPUT_MOUSE;

    // 根据全局配置判断使用左键还是右键的标志位
    DWORD down_flag = (action_button == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    DWORD up_flag   = (action_button == 0) ? MOUSEEVENTF_LEFTUP   : MOUSEEVENTF_RIGHTUP;

    // 根据全局配置判断执行次数（0:单击则执行1次，1:双击则执行2次）
    int loop_count = (action_mode == 0) ? 1 : 2;

    for (int i = 0; i < loop_count; i++) {
        // 执行按下动作
        input.mi.dwFlags = down_flag;
        SendInput(1, &input, sizeof(INPUT));

        // 执行抬起动作
        input.mi.dwFlags = up_flag;
        SendInput(1, &input, sizeof(INPUT));

        // 如果是双击模式，且是第一次点击完毕，则增加一个极其微小的系统级延迟，确保双击能被系统正确识别
        if (loop_count > 1 && i == 0) {
            Sleep(30); 
        }
    }
}
