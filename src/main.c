#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// 引入自定义头文件
#include "globals.h"
#include "worker.h"
#include "resource.h"

// ====== 启用 Windows 现代视觉样式 ======
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
// =======================================

// 实例化原有全局变量
bool is_active = false;
int interval_min = 30;
int interval_max = 30;
HWND hStatusLabel = NULL;

// 实例化原有功能配置变量
int action_button = 0; // 0: 左键, 1: 右键
int action_mode = 0;   // 0: 单击, 1: 双击
int hotkey_toggle = VK_F8; // 默认 F8
int hotkey_stop = VK_F9;   // 默认 F9

// 实例化原有后台绑定相关变量
HWND target_hwnd = NULL;
POINT bind_pt = {0, 0};
int hotkey_bind = VK_F10;  // 默认 F10
HWND hBindLabel = NULL;

// ====== 新增：实例化录制与回放相关全局变量 ======
MouseRecord record_buffer[MAX_RECORD_SIZE];
int record_count = 0;
bool is_recording = false;
int playback_count = 1;
int hotkey_record = VK_F11;   // 默认 F11
int hotkey_playback = VK_F12; // 默认 F12

// 局部 UI 控件句柄
HWND hEditMin, hEditMax, hBtnApply;
HWND hCmbBtnType, hCmbActType, hCmbHkToggle, hCmbHkStop, hCmbHkBind;

// ====== 新增：录制与回放 UI 控件句柄 ======
HWND hEditPlayCount, hCmbHkRecord, hCmbHkPlay;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "SysToolWindowClass";
    
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); 
    
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc)) return 0;

    // 调整窗口高度以容纳新的录制回放设置行 (高度从 280 增加至 330)
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "SysTool", 
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 360, 330,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void SetDefaultFont(HWND hwnd) {
    SendMessage(hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

void PopulateHotkeyCombo(HWND hCombo) {
    const char* keys[] = {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"};
    for (int i = 0; i < 12; i++) {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)keys[i]);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // 第一行：间隔设置
            HWND hLbl1 = CreateWindow("STATIC", "最小间隔:", WS_VISIBLE | WS_CHILD, 15, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hEditMin = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 95, 12, 65, 20, hwnd, (HMENU)ID_EDIT_MIN, NULL, NULL);
            
            HWND hLbl2 = CreateWindow("STATIC", "最大间隔:", WS_VISIBLE | WS_CHILD, 175, 15, 80, 20, hwnd, NULL, NULL, NULL);
            hEditMax = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 255, 12, 65, 20, hwnd, (HMENU)ID_EDIT_MAX, NULL, NULL);

            // 第二行：按键与模式
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

            // 第三行：控制热键
            HWND hLbl5 = CreateWindow("STATIC", "开启热键:", WS_VISIBLE | WS_CHILD, 15, 75, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkToggle = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 72, 65, 150, hwnd, (HMENU)ID_CMB_HK_TOGGLE, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkToggle);
            SendMessage(hCmbHkToggle, CB_SETCURSEL, 7, 0); // F8

            HWND hLbl6 = CreateWindow("STATIC", "停止热键:", WS_VISIBLE | WS_CHILD, 175, 75, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkStop = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 255, 72, 65, 150, hwnd, (HMENU)ID_CMB_HK_STOP, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkStop);
            SendMessage(hCmbHkStop, CB_SETCURSEL, 8, 0); // F9

            // 第四行：绑定热键 与 录制热键并排
            HWND hLbl7 = CreateWindow("STATIC", "绑定热键:", WS_VISIBLE | WS_CHILD, 15, 105, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkBind = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 102, 65, 150, hwnd, (HMENU)ID_CMB_HK_BIND, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkBind);
            SendMessage(hCmbHkBind, CB_SETCURSEL, 9, 0); // F10

            HWND hLbl8 = CreateWindow("STATIC", "录制热键:", WS_VISIBLE | WS_CHILD, 175, 105, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkRecord = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 255, 102, 65, 150, hwnd, (HMENU)ID_CMB_HK_RECORD, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkRecord);
            SendMessage(hCmbHkRecord, CB_SETCURSEL, 10, 0); // F11

            // 第五行：新增 回放热键 与 回放次数并排
            HWND hLbl9 = CreateWindow("STATIC", "回放热键:", WS_VISIBLE | WS_CHILD, 15, 135, 80, 20, hwnd, NULL, NULL, NULL);
            hCmbHkPlay = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 95, 132, 65, 150, hwnd, (HMENU)ID_CMB_HK_PLAY, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkPlay);
            SendMessage(hCmbHkPlay, CB_SETCURSEL, 11, 0); // F12

            HWND hLbl10 = CreateWindow("STATIC", "回放次数:", WS_VISIBLE | WS_CHILD, 175, 135, 80, 20, hwnd, NULL, NULL, NULL);
            hEditPlayCount = CreateWindow("EDIT", "1", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 255, 132, 65, 20, hwnd, (HMENU)ID_EDIT_PLAYCOUNT, NULL, NULL);

            // 第六行：应用按钮 (Y坐标由 140 下移至 170)
            hBtnApply = CreateWindow("BUTTON", "应用所有设置", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 15, 170, 305, 30, hwnd, (HMENU)ID_BTN_APPLY, NULL, NULL);
            
            // 第七行：运行状态 (Y坐标由 185 下移至 215)
            hStatusLabel = CreateWindow("STATIC", ">> 状态: 已准备就绪", WS_VISIBLE | WS_CHILD, 15, 215, 305, 20, hwnd, NULL, NULL, NULL);
            
            // 第八行：绑定状态 (Y坐标由 210 下移至 240)
            hBindLabel = CreateWindow("STATIC", "未绑定 (全局模式)", WS_VISIBLE | WS_CHILD, 15, 240, 305, 20, hwnd, NULL, NULL, NULL);
            
            // 字体美化
            SetDefaultFont(hLbl1); SetDefaultFont(hEditMin);
            SetDefaultFont(hLbl2); SetDefaultFont(hEditMax);
            SetDefaultFont(hLbl3); SetDefaultFont(hCmbBtnType);
            SetDefaultFont(hLbl4); SetDefaultFont(hCmbActType);
            SetDefaultFont(hLbl5); SetDefaultFont(hCmbHkToggle);
            SetDefaultFont(hLbl6); SetDefaultFont(hCmbHkStop);
            SetDefaultFont(hLbl7); SetDefaultFont(hCmbHkBind);
            SetDefaultFont(hLbl8); SetDefaultFont(hCmbHkRecord);
            SetDefaultFont(hLbl9); SetDefaultFont(hCmbHkPlay);
            SetDefaultFont(hLbl10); SetDefaultFont(hEditPlayCount);
            SetDefaultFont(hBtnApply); SetDefaultFont(hStatusLabel); SetDefaultFont(hBindLabel);

            CreateThread(NULL, 0, WorkerThread, (LPVOID)hwnd, 0, NULL);
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE);
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_APPLY) {
                // 原有参数读取
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

                // ====== 新增：录制回放参数读取 ======
                char szPlay[16];
                GetWindowText(hEditPlayCount, szPlay, 16);
                int temp_play = atoi(szPlay);
                if (temp_play < 1) temp_play = 1;
                sprintf(szPlay, "%d", temp_play);
                SetWindowText(hEditPlayCount, szPlay);
                playback_count = temp_play;

                hotkey_record   = VK_F1 + SendMessage(hCmbHkRecord, CB_GETCURSEL, 0, 0);
                hotkey_playback = VK_F1 + SendMessage(hCmbHkPlay, CB_GETCURSEL, 0, 0);

                MessageBox(hwnd, "参数与快捷键已成功应用！\n(可使用新增的录制与回放热键进行操作)", "提示", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
