// ==================== src/worker.c ====================
// 替换整个文件内容（覆盖原代码）

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "globals.h"
#include "worker.h"
#include "resource.h"
#include "action.h"

#define _CRT_SECURE_NO_WARNINGS

// ====================== UI 控件句柄（从主窗口复制） ======================
extern HWND hEditMin, hEditMax, hBtnApply;
extern HWND hCmbBtnType, hCmbActType, hCmbHkToggle, hCmbHkStop, hCmbHkBind;
extern HWND hStatusLabel;
extern HWND hBindLabel;

// ====================== 全局变量 ======================
bool is_recording = false;
bool is_replaying = false;
int replay_repeats = 1;
unsigned long long replay_start_time = 0;
int MAX_POINTS = 5000;
typedef struct {
    int x, y;
    unsigned long long timestamp;
} Point;
Point *points = NULL;
int current_points = 0;
int replay_index = 0;

// ====================== 字体/热键下拉框函数（保留） ======================
void SetDefaultFont(HWND hwnd) {
    SendMessage(hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

void PopulateHotkeyCombo(HWND hCombo) {
    const char* keys[] = {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"};
    for (int i = 0; i < 12; i++) {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)keys[i]);
    }
}

// ====================== 录制/回放专用函数 ======================
void start_recording() {
    if (is_recording) return;
    is_recording = true;
    current_points = 0;
    if (points) free(points);
    points = (Point*)malloc(sizeof(Point) * MAX_POINTS);
    SetWindowTextA(hStatusLabel, ">> 状态: 录制中... 按 P 暂停，C 取消，R 重新录制");
    MessageBox(NULL, "鼠标轨迹录制已开始！\n移动鼠标即可录制路径（鼠标需悬停在窗口上）。\n按 P 暂停/继续，按 C 取消，按 R 重新开始。", "鼠标轨迹录制", MB_OK | MB_ICONINFORMATION);
}

void stop_recording() {
    if (!is_recording) return;
    is_recording = false;
    SetWindowTextA(hStatusLabel, ">> 状态: 录制已停止（轨迹已保存）");
}

void replay_trajectory() {
    if (!is_recording || current_points == 0) {
        MessageBox(NULL, "请先录制一条轨迹！", "提示", MB_OK | MB_ICONWARNING);
        return;
    }
    is_replaying = true;
    replay_index = 0;
    replay_start_time = GetTickCount64();
    SetWindowTextA(hStatusLabel, ">> 状态: 回放中...");

    // 正确读取回放次数（从主窗口的间隔编辑框）
    char input[10];
    GetWindowTextA(hEditMax, input, sizeof(input));
    replay_repeats = atoi(input);
    if (replay_repeats <= 0) replay_repeats = 1;

    SetWindowTextA(hStatusLabel, "回放中...");
    MessageBox(NULL, "回放已启动！按 B 停止回放。", "回放提示", MB_OK | MB_ICONINFORMATION);
}

void stop_replay() {
    is_replaying = false;
    SetWindowTextA(hStatusLabel, ">> 状态: 回放已停止");
}

// ====================== 窗口消息处理 ======================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // ==================== 原间隔设置控件（保留） ====================
            HWND hLbl1 = CreateWindow("STATIC", "最小间隔:", WS_VISIBLE | WS_CHILD, 15, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hEditMin = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 95, 12, 65, 20, hwnd, (HMENU)ID_EDIT_MIN, NULL, NULL);
            
            HWND hLbl2 = CreateWindow("STATIC", "最大间隔:", WS_VISIBLE | WS_CHILD, 175, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hEditMax = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 255, 12, 65, 20, hwnd, (HMENU)ID_EDIT_MAX, NULL, NULL);

            // ==================== 新增：回放次数设置控件（这是你最需要修复的部分） ====================
            HWND hLblReplay = CreateWindow("STATIC", "回放次数:", WS_VISIBLE | WS_CHILD, 15, 195, 80, 20, hwnd, NULL, NULL, NULL);
            HWND hEditReplay = CreateWindow("EDIT", "1", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 255, 192, 65, 20, hwnd, (HMENU)ID_EDIT_MAX, NULL, NULL);  // 用 ID_EDIT_MAX（主窗口里也用了这个ID）

            // ==================== 原热键/模式控件（保留） ====================
            HWND hLbl3 = CreateWindow("STATIC", "模拟按键:", WS_VISIBLE | WS_CHILD, 15, 45, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbBtnType = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 42, 65, 150, hwnd, (HMENU)ID_CMB_BTN_TYPE, NULL, NULL);
            SendMessage(hCmbBtnType, CB_ADDSTRING, 0, (LPARAM)"左键");
            SendMessage(hCmbBtnType, CB_ADDSTRING, 0, (LPARAM)"右键");
            SendMessage(hCmbBtnType, CB_SETCURSEL, 0, 0);

            HWND hLbl4 = CreateWindow("STATIC", "点击模式:", WS_VISIBLE | WS_CHILD, 175, 45, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbActType = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 255, 42, 65, 150, hwnd, (HMENU)ID_CMB_ACT_TYPE, NULL, NULL);
            SendMessage(hCmbActType, CB_ADDSTRING, 0, (LPARAM)"单击");
            SendMessage(hCmbActType, CB_ADDSTRING, 0, (LPARAM)"双击");
            SendMessage(hCmbActType, CB_SETCURSEL, 0, 0);

            HWND hLbl5 = CreateWindow("STATIC", "开启热键:", WS_VISIBLE | WS_CHILD, 15, 75, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkToggle = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 72, 65, 150, hwnd, (HMENU)ID_CMB_HK_TOGGLE, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkToggle);
            SendMessage(hCmbHkToggle, CB_SETCURSEL, 7, 0);

            HWND hLbl6 = CreateWindow("STATIC", "停止热键:", WS_VISIBLE | WS_CHILD, 175, 75, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkStop = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 255, 72, 65, 150, hwnd, (HMENU)ID_CMB_HK_STOP, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkStop);
            SendMessage(hCmbHkStop, CB_SETCURSEL, 8, 0);

            HWND hLbl7 = CreateWindow("STATIC", "绑定热键:", WS_VISIBLE | WS_CHILD, 15, 105, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkBind = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 102, 65, 150, hwnd, (HMENU)ID_CMB_HK_BIND, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkBind);
            SendMessage(hCmbHkBind, CB_SETCURSEL, 9, 0);

            hBtnApply = CreateWindow("BUTTON", "应用所有设置", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 15, 140, 305, 30, hwnd, (HMENU)ID_BTN_APPLY, NULL, NULL);
            
            hStatusLabel = CreateWindow("STATIC", ">> 状态: 已准备就绪", WS_VISIBLE | WS_CHILD, 15, 185, 305, 20, hwnd, NULL, NULL, NULL);
            hBindLabel = CreateWindow("STATIC", "未绑定 (全局模式)", WS_VISIBLE | WS_CHILD, 15, 210, 305, 20, hwnd, NULL, NULL, NULL);

            SetDefaultFont(hLbl1); SetDefaultFont(hEditMin); SetDefaultFont(hLbl2); SetDefaultFont(hEditMax);
            SetDefaultFont(hLblReplay); SetDefaultFont(hEditReplay);  // 新增
            SetDefaultFont(hLbl3); SetDefaultFont(hCmbBtnType); SetDefaultFont(hLbl4); SetDefaultFont(hCmbActType);
            SetDefaultFont(hLbl5); SetDefaultFont(hCmbHkToggle); SetDefaultFont(hLbl6); SetDefaultFont(hCmbHkStop);
            SetDefaultFont(hLbl7); SetDefaultFont(hCmbHkBind); SetDefaultFont(hBtnApply); SetDefaultFont(hStatusLabel); SetDefaultFont(hBindLabel);

            CreateThread(NULL, 0, WorkerThread, (LPVOID)hwnd, 0, NULL);
            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_APPLY) {
                // 原代码（保留）
                char szMin[16], szMax[16];
                GetWindowText(hEditMin, szMin, 16);
                GetWindowText(hEditMax, szMax, 16);
                int temp_min = atoi(szMin);
                int temp_max = atoi(szMax);
                if (temp_min < 1) temp_min = 1;
                if (temp_max < temp_min) temp_max = temp_min;
                sprintf(szMin, "%d", temp_min);
                sprintf(szMax, "%d", temp_max);
                SetWindowText(hEditMin, szMin);
                SetWindowText(hEditMax, szMax);
                
                interval_min = temp_min;
                interval_max = temp_max;

                action_button = SendMessage(hCmbBtnType, CB_GETCURSEL, 0, 0);
                action_mode   = SendMessage(hCmbActType, CB_GETCURSEL, 0, 0);
                hotkey_toggle = VK_F1 + SendMessage(hCmbHkToggle, CB_GETCURSEL, 0, 0);
                hotkey_stop   = VK_F1 + SendMessage(hCmbHkStop, CB_GETCURSEL, 0, 0);
                hotkey_bind   = VK_F1 + SendMessage(hCmbHkBind, CB_GETCURSEL, 0, 0);

                MessageBox(hwnd, "参数与快捷键已成功应用！\n(鼠标悬停在目标位置按下绑定键即可后台锁定)", "提示", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }

        case WM_KEYDOWN: {
            switch (wParam) {
                case 'R': start_recording(); break;
                case 'P': 
                    if (is_recording) {
                        is_recording = false;
                        SetWindowTextA(hStatusLabel, ">> 状态: 录制已暂停（按 R 继续录制）");
                    } else if (current_points > 0) {
                        is_recording = true;
                        SetWindowTextA(hStatusLabel, ">> 状态: 录制已恢复");
                    }
                    break;
                case 'C': stop_recording(); break;
                case 'B': 
                    if (is_recording) stop_recording();
                    replay_trajectory();
                    break;
            }
            break;
        }

        case WM_CLOSE: DestroyWindow(hwnd); break;
        case WM_DESTROY: PostQuitMessage(0); break;
        default: return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ====================== 线程入口（修复版） ======================
DWORD WINAPI WorkerThread(LPVOID lpParam) {
    HWND hMainWnd = (HWND)lpParam;
    srand((unsigned int)time(NULL));

    while (1) {
        // 原热键侦测（保留）
        if (GetAsyncKeyState(hotkey_bind) & 0x8000) {
            // 绑定/解绑逻辑（原代码保持不变）
            POINT pt;
            GetCursorPos(&pt);
            HWND hwndUnderCursor = WindowFromPoint(pt);
            if (hwndUnderCursor != NULL && hwndUnderCursor != hMainWnd) {
                ScreenToClient(hwndUnderCursor, &pt);
                target_hwnd = hwndUnderCursor;
                bind_pt = pt;
                char title[128];
                GetWindowTextA(target_hwnd, title, sizeof(title));
                if (strlen(title) == 0) sprintf(title, "%p", target_hwnd);
                SetWindowTextA(hBindLabel, title);
            } else if (target_hwnd != NULL) {
                target_hwnd = NULL;
                SetWindowTextA(hBindLabel, "未绑定 (全局模式)");
            }
            Sleep(300);
        }
        if (GetAsyncKeyState(hotkey_stop) & 0x8000) {
            // 停止逻辑（原代码保持不变）
            if (is_active) is_active = false;
            else is_active = true;
            Sleep(300);
        }
        if (GetAsyncKeyState(hotkey_toggle) & 0x8000) {
            // 开启/暂停逻辑（原代码保持不变）
            if (is_active) is_active = false;
            else is_active = true;
            Sleep(300);
        }

        // 执行动作（原代码）
        if (is_active) {
            execute_action();
            int current_interval = interval_min;
            if (interval_max > interval_min) {
                current_interval = interval_min + rand() % (interval_max - interval_min + 1);
            }
            Sleep(current_interval);
        } else {
            Sleep(10);
        }

        // ====================== 录制/回放（已修复） ======================
        if (is_recording) {
            POINT pt;
            if (GetCursorPos(&pt) && current_points < MAX_POINTS) {
                points[current_points].x = pt.x;
                points[current_points].y = pt.y;
                points[current_points].timestamp = GetTickCount64() - replay_start_time;
                current_points++;
            }
        }

        if (is_replaying && current_points > 0) {
            if (replay_index < current_points) {
                unsigned long long elapsed = GetTickCount64() - replay_start_time;
                while (GetTickCount64() - replay_start_time < elapsed) Sleep(1);
                
                INPUT input = {0};
                input.type = INPUT_MOUSE;
                input.mi.dx = points[replay_index].x;
                input.mi.dy = points[replay_index].y;
                input.mi.dwFlags = MOUSEEVENTF_MOVE;
                SendInput(1, &input, sizeof(INPUT));
                replay_index++;
            } else {
                if (replay_repeats > 1) {
                    replay_repeats--;
                    replay_index = 0;
                } else {
                    stop_replay();
                }
            }
        }
    }
    return 0;
}
