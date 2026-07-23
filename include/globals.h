#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <stdbool.h>

// UI 控件 ID
#define ID_BTN_APPLY      1001
#define ID_EDIT_MIN       1002
#define ID_EDIT_MAX       1003
#define ID_CMB_BTN_TYPE   1004
#define ID_CMB_ACT_TYPE   1005
#define ID_CMB_HK_TOGGLE  1006
#define ID_CMB_HK_STOP    1007
#define ID_CMB_HK_BIND    1008
#define ID_EDIT_PLAYCOUNT 1009
#define ID_CMB_HK_RECORD  1010
#define ID_CMB_HK_PLAY    1011

// 全局热键 ID
#define HOTKEY_ID_TOGGLE   2001
#define HOTKEY_ID_STOP     2002
#define HOTKEY_ID_BIND     2003
#define HOTKEY_ID_RECORD   2004
#define HOTKEY_ID_PLAYBACK 2005

#define MAX_RECORD_SIZE   1024

typedef struct {
    POINT pt;
    UINT msg_type;
    DWORD delay;
} MouseRecord;

// 全局变量
extern bool is_active;
extern int interval_min;
extern int interval_max;
extern HWND hStatusLabel;

extern int action_button;
extern int action_mode;
extern int hotkey_toggle;
extern int hotkey_stop;

extern HWND target_hwnd;
extern POINT bind_pt;
extern int hotkey_bind;
extern HWND hBindLabel;

extern MouseRecord record_buffer[MAX_RECORD_SIZE];
extern int record_count;
extern bool is_recording;
extern int playback_count;
extern int hotkey_record;
extern int hotkey_playback;

#endif
