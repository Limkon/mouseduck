#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <stdbool.h>

// UI 控件 ID 宏定义
#define ID_BTN_APPLY      1001
#define ID_EDIT_MIN       1002
#define ID_EDIT_MAX       1003
#define ID_CMB_BTN_TYPE   1004
#define ID_CMB_ACT_TYPE   1005
#define ID_CMB_HK_TOGGLE  1006
#define ID_CMB_HK_STOP    1007
#define ID_CMB_HK_BIND    1008  // 新增：绑定热键下拉框 ID

// 全局变量声明：跨线程通信与状态同步
extern bool is_active;
extern int interval_min;
extern int interval_max;
extern HWND hStatusLabel;

// 原有功能配置变量
extern int action_button; // 0: 左键, 1: 右键
extern int action_mode;   // 0: 单击, 1: 双击
extern int hotkey_toggle; 
extern int hotkey_stop;   

// ====== 新增：后台绑定相关变量 ======
extern HWND target_hwnd;  // 绑定的目标窗口句柄 (NULL代表未绑定全局模式)
extern POINT bind_pt;     // 绑定时锁定的窗口内相对坐标
extern int hotkey_bind;   // 绑定/解绑热键 (默认 VK_F10)
extern HWND hBindLabel;   // UI 上显示当前绑定状态的标签句柄

#endif // GLOBALS_H
