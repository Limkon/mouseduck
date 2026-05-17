#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <stdbool.h>

// UI 控件 ID 宏定义
#define ID_BTN_APPLY      1001
#define ID_EDIT_MIN       1002
#define ID_EDIT_MAX       1003
#define ID_CMB_BTN_TYPE   1004  // 按键类型 (左/右)
#define ID_CMB_ACT_TYPE   1005  // 动作类型 (单/双)
#define ID_CMB_HK_TOGGLE  1006  // 开启/暂停热键
#define ID_CMB_HK_STOP    1007  // 停止热键

// 全局变量声明：跨线程通信与状态同步
extern bool is_active;
extern int interval_min;
extern int interval_max;
extern HWND hStatusLabel;

// 新增功能配置变量
extern int action_button; // 0: 左键, 1: 右键
extern int action_mode;   // 0: 单击, 1: 双击
extern int hotkey_toggle; // 开启/暂停的虚拟键码 (默认 VK_F8)
extern int hotkey_stop;   // 停止的虚拟键码 (默认 VK_F9)

#endif // GLOBALS_H
