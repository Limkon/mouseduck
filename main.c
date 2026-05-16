#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

// 执行核心动作的抽象函数
void execute_action() {
    INPUT input = {0};
    
    // 按下动作
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    // 清理结构体并执行抬起动作
    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

int main() {
    // 设置非敏感的控制台标题
    SetConsoleTitleA("SysTool");
    
    printf("系统辅助工具已加载。\n");
    printf("========================\n");
    printf("[热键 F8] : 开启 / 暂停循环任务\n");
    printf("[热键 F9] : 完全结束并退出工具\n");
    printf("========================\n");

    bool is_active = false;
    int interval_ms = 30; // 默认执行间隔（毫秒）

    // 主消息与热键监听循环
    while (1) {
        // 侦测退出热键 (F9)
        if (GetAsyncKeyState(VK_F9) & 0x8000) {
            printf("\n收到退出指令，正在终止进程...\n");
            break;
        }

        // 侦测状态切换热键 (F8)
        if (GetAsyncKeyState(VK_F8) & 0x8000) {
            is_active = !is_active;
            if (is_active) {
                printf(">> 任务状态：已开启\n");
            } else {
                printf(">> 任务状态：已暂停\n");
            }
            // 防抖动延迟，避免单次按键触发多次切换
            Sleep(300); 
        }

        // 执行动作逻辑
        if (is_active) {
            execute_action();
            Sleep(interval_ms);
        } else {
            // 暂停状态下释放 CPU 资源
            Sleep(10); 
        }
    }

    return 0;
}
