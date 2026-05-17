#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <stdbool.h>

// UI 控件 ID 宏定义
#define ID_BTN_APPLY 1001
#define ID_EDIT_MIN  1002
#define ID_EDIT_MAX  1003

// 全局变量声明：跨线程通信
extern bool is_active;
extern int interval_min;
extern int interval_max;
extern HWND hStatusLabel;

#endif // GLOBALS_H
