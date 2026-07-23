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
#define ID_CMB_HK_BIND    1008  // 原有：绑定热键下拉框 ID

// ====== 新增：录制与回放相关宏与 UI ID ======
#define MAX_RECORD_SIZE   1024
#define ID_EDIT_PLAYCOUNT 1009
#define ID_CMB_HK_RECORD  1010
#define ID_CMB_HK_PLAY    1011

// ====== 新增：鼠标动作结构体 ======
typedef struct {
    POINT pt;       // 屏幕绝对坐标
    UINT msg_type;  // 动作类型 (如 WM_LBUTTONDOWN 等)
    DWORD delay;    // 距离上一个动作的毫秒延迟时间
} MouseRecord;

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

// ====== 原有：后台绑定相关变量 ======
extern HWND target_hwnd;  // 绑定的目标窗口句柄 (NULL代表未绑定全局模式)
extern POINT bind_pt;     // 绑定时锁定的窗口内相对坐标
extern int hotkey_bind;   // 绑定/解绑热键 (默认 VK_F10)
extern HWND hBindLabel;   // UI 上显示当前绑定状态的标签句柄

// ====== 新增：录制与回放全局变量 ======
extern MouseRecord record_buffer[MAX_RECORD_SIZE];
extern int record_count;
extern bool is_recording;
extern int playback_count;
extern int hotkey_record;
extern int hotkey_playback;

#endif // GLOBALS_H
