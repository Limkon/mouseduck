#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "worker.h"
#include "action.h"     // 新增：解决 execute_playback 未定义
#include "resource.h"

bool is_active = false;
int interval_min = 30, interval_max = 30;
HWND hStatusLabel = NULL;

int action_button = 0, action_mode = 0;
int hotkey_toggle = VK_F8, hotkey_stop = VK_F9;

HWND target_hwnd = NULL;
POINT bind_pt = {0, 0};
int hotkey_bind = VK_F10;
HWND hBindLabel = NULL;

MouseRecord record_buffer[MAX_RECORD_SIZE];
int record_count = 0;
bool is_recording = false;
int playback_count = 1;
int hotkey_record = VK_F11, hotkey_playback = VK_F12;

HWND hEditMin, hEditMax, hBtnApply;
HWND hCmbBtnType, hCmbActType, hCmbHkToggle, hCmbHkStop, hCmbHkBind;
HWND hEditPlayCount, hCmbHkRecord, hCmbHkPlay;

// 先声明
BOOL RegisterAllHotkeys(HWND hwnd);
void UnregisterAllHotkeys(HWND hwnd);

BOOL RegisterAllHotkeys(HWND hwnd) {
    UnregisterAllHotkeys(hwnd);
    RegisterHotKey(hwnd, HOTKEY_ID_TOGGLE, 0, hotkey_toggle);
    RegisterHotKey(hwnd, HOTKEY_ID_STOP, 0, hotkey_stop);
    RegisterHotKey(hwnd, HOTKEY_ID_BIND, 0, hotkey_bind);
    RegisterHotKey(hwnd, HOTKEY_ID_RECORD, 0, hotkey_record);
    RegisterHotKey(hwnd, HOTKEY_ID_PLAYBACK, 0, hotkey_playback);
    return TRUE;
}

void UnregisterAllHotkeys(HWND hwnd) {
    UnregisterHotKey(hwnd, HOTKEY_ID_TOGGLE);
    UnregisterHotKey(hwnd, HOTKEY_ID_STOP);
    UnregisterHotKey(hwnd, HOTKEY_ID_BIND);
    UnregisterHotKey(hwnd, HOTKEY_ID_RECORD);
    UnregisterHotKey(hwnd, HOTKEY_ID_PLAYBACK);
}

void SetDefaultFont(HWND h) {
    SendMessage(h, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

void PopulateHotkeyCombo(HWND hCombo) {
    const char* keys[] = {"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12"};
    for (int i = 0; i < 12; i++) SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)keys[i]);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            CreateWindow("STATIC", "最小间隔:", WS_VISIBLE | WS_CHILD, 15, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hEditMin = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 95, 12, 65, 20, hwnd, (HMENU)ID_EDIT_MIN, NULL, NULL);
            CreateWindow("STATIC", "最大间隔:", WS_VISIBLE | WS_CHILD, 175, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hEditMax = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 255, 12, 65, 20, hwnd, (HMENU)ID_EDIT_MAX, NULL, NULL);

            CreateWindow("STATIC", "模拟按键:", WS_VISIBLE | WS_CHILD, 15, 45, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbBtnType = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 42, 65, 150, hwnd, (HMENU)ID_CMB_BTN_TYPE, NULL, NULL);
            SendMessage(hCmbBtnType, CB_ADDSTRING, 0, (LPARAM)"左键");
            SendMessage(hCmbBtnType, CB_ADDSTRING, 0, (LPARAM)"右键");
            SendMessage(hCmbBtnType, CB_SETCURSEL, 0, 0);
            CreateWindow("STATIC", "点击模式:", WS_VISIBLE | WS_CHILD, 175, 45, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbActType = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 255, 42, 65, 150, hwnd, (HMENU)ID_CMB_ACT_TYPE, NULL, NULL);
            SendMessage(hCmbActType, CB_ADDSTRING, 0, (LPARAM)"单击");
            SendMessage(hCmbActType, CB_ADDSTRING, 0, (LPARAM)"双击");
            SendMessage(hCmbActType, CB_SETCURSEL, 0, 0);

            CreateWindow("STATIC", "开启热键:", WS_VISIBLE | WS_CHILD, 15, 75, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkToggle = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 72, 65, 150, hwnd, (HMENU)ID_CMB_HK_TOGGLE, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkToggle); SendMessage(hCmbHkToggle, CB_SETCURSEL, 7, 0);
            CreateWindow("STATIC", "停止热键:", WS_VISIBLE | WS_CHILD, 175, 75, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkStop = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 255, 72, 65, 150, hwnd, (HMENU)ID_CMB_HK_STOP, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkStop); SendMessage(hCmbHkStop, CB_SETCURSEL, 8, 0);

            CreateWindow("STATIC", "绑定热键:", WS_VISIBLE | WS_CHILD, 15, 105, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkBind = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 102, 65, 150, hwnd, (HMENU)ID_CMB_HK_BIND, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkBind); SendMessage(hCmbHkBind, CB_SETCURSEL, 9, 0);
            CreateWindow("STATIC", "录制热键:", WS_VISIBLE | WS_CHILD, 175, 105, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkRecord = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 255, 102, 65, 150, hwnd, (HMENU)ID_CMB_HK_RECORD, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkRecord); SendMessage(hCmbHkRecord, CB_SETCURSEL, 10, 0);

            CreateWindow("STATIC", "回放热键:", WS_VISIBLE | WS_CHILD, 15, 135, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkPlay = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 132, 65, 150, hwnd, (HMENU)ID_CMB_HK_PLAY, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkPlay); SendMessage(hCmbHkPlay, CB_SETCURSEL, 11, 0);
            CreateWindow("STATIC", "回放次数:", WS_VISIBLE | WS_CHILD, 175, 135, 80, 20, hwnd, NULL, NULL, NULL);
            hEditPlayCount = CreateWindow("EDIT", "1", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 255, 132, 65, 20, hwnd, (HMENU)ID_EDIT_PLAYCOUNT, NULL, NULL);

            hBtnApply = CreateWindow("BUTTON", "应用所有设置", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 15, 170, 305, 30, hwnd, (HMENU)ID_BTN_APPLY, NULL, NULL);
            hStatusLabel = CreateWindow("STATIC", ">> 状态: 已准备就绪", WS_VISIBLE | WS_CHILD, 15, 215, 305, 20, hwnd, NULL, NULL, NULL);
            hBindLabel = CreateWindow("STATIC", "未绑定 (全局模式)", WS_VISIBLE | WS_CHILD, 15, 240, 305, 20, hwnd, NULL, NULL, NULL);

            HWND ctrls[] = {hEditMin,hEditMax,hCmbBtnType,hCmbActType,hCmbHkToggle,hCmbHkStop,
                            hCmbHkBind,hCmbHkRecord,hCmbHkPlay,hEditPlayCount,hBtnApply,hStatusLabel,hBindLabel};
            for (int i = 0; i < (int)(sizeof(ctrls)/sizeof(ctrls[0])); i++) SetDefaultFont(ctrls[i]);

            CreateThread(NULL, 0, WorkerThread, hwnd, 0, NULL);
            RegisterAllHotkeys(hwnd);
            break;
        }

        case WM_HOTKEY: {
            int id = (int)wParam;
            if (id == HOTKEY_ID_RECORD) {
                is_recording = !is_recording;
                SetWindowTextA(hStatusLabel, is_recording ? ">> 状态: 正在录制中..." : ">> 状态: 录制完成");
                if (is_recording) record_count = 0;
            } else if (id == HOTKEY_ID_PLAYBACK) {
                if (!is_recording && record_count > 0) {
                    SetWindowTextA(hStatusLabel, ">> 状态: 正在回放中...");
                    execute_playback();
                    SetWindowTextA(hStatusLabel, ">> 状态: 已准备就绪");
                }
            } else if (id == HOTKEY_ID_TOGGLE) {
                is_active = !is_active;
                SetWindowTextA(hStatusLabel, is_active ? ">> 状态: 运行中..." : ">> 状态: 已暂停");
            } else if (id == HOTKEY_ID_STOP) {
                is_active = false;
                SetWindowTextA(hStatusLabel, ">> 状态: 已强制停止");
            }
            return 0;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BTN_APPLY) {
                char buf[16];
                GetWindowText(hEditMin, buf, 16); interval_min = atoi(buf);
                GetWindowText(hEditMax, buf, 16); interval_max = atoi(buf);
                if (interval_min < 1) interval_min = 1;
                if (interval_max < interval_min) interval_max = interval_min;

                action_button = (int)SendMessage(hCmbBtnType, CB_GETCURSEL, 0, 0);
                action_mode = (int)SendMessage(hCmbActType, CB_GETCURSEL, 0, 0);

                hotkey_toggle = VK_F1 + (int)SendMessage(hCmbHkToggle, CB_GETCURSEL, 0, 0);
                hotkey_stop   = VK_F1 + (int)SendMessage(hCmbHkStop, CB_GETCURSEL, 0, 0);
                hotkey_bind   = VK_F1 + (int)SendMessage(hCmbHkBind, CB_GETCURSEL, 0, 0);

                GetWindowText(hEditPlayCount, buf, 16);
                playback_count = atoi(buf);
                if (playback_count < 1) playback_count = 1;

                hotkey_record   = VK_F1 + (int)SendMessage(hCmbHkRecord, CB_GETCURSEL, 0, 0);
                hotkey_playback = VK_F1 + (int)SendMessage(hCmbHkPlay, CB_GETCURSEL, 0, 0);

                RegisterAllHotkeys(hwnd);
                MessageBox(hwnd, "设置已应用！\n全局热键已生效", "成功", MB_OK | MB_ICONINFORMATION);
            }
            break;

        case WM_CLOSE:
            UnregisterAllHotkeys(hwnd);
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            UnregisterAllHotkeys(hwnd);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "SysToolWindowClass";
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hIcon = wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc)) return 0;

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, "mouseduck", 
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 360, 330, NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
