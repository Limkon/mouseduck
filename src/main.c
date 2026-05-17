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

// 实例化全局变量
bool is_active = false;
int interval_min = 30;
int interval_max = 30;
HWND hStatusLabel = NULL;

// 实例化新增功能全局变量及默认值
int action_button = 0; // 0: 左键, 1: 右键
int action_mode = 0;   // 0: 单击, 1: 双击
int hotkey_toggle = VK_F8; // 默认 F8
int hotkey_stop = VK_F9;   // 默认 F9

// 局部 UI 控件句柄
HWND hEditMin, hEditMax, hBtnApply;
HWND hCmbBtnType, hCmbActType, hCmbHkToggle, hCmbHkStop;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "SysToolWindowClass";
    
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); // 修复色差
    
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc)) return 0;

    // 拉高主窗口以容纳更多控件
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "SysTool", 
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 360,
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

// 辅助函数：向下拉菜单填充 F1 - F12
void PopulateHotkeyCombo(HWND hCombo) {
    const char* keys[] = {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"};
    for (int i = 0; i < 12; i++) {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)keys[i]);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // 1. 间隔设置区域
            HWND hLbl1 = CreateWindow("STATIC", "最小间隔 (ms):", WS_VISIBLE | WS_CHILD, 20, 20, 100, 20, hwnd, NULL, NULL, NULL);
            hEditMin = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 130, 20, 80, 20, hwnd, (HMENU)ID_EDIT_MIN, NULL, NULL);
            
            HWND hLbl2 = CreateWindow("STATIC", "最大间隔 (ms):", WS_VISIBLE | WS_CHILD, 20, 55, 100, 20, hwnd, NULL, NULL, NULL);
            hEditMax = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 130, 55, 80, 20, hwnd, (HMENU)ID_EDIT_MAX, NULL, NULL);

            // 2. 动作选择区域
            HWND hLbl3 = CreateWindow("STATIC", "模拟按键:", WS_VISIBLE | WS_CHILD, 20, 90, 100, 20, hwnd, NULL, NULL, NULL);
            hCmbBtnType = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 130, 90, 80, 100, hwnd, (HMENU)ID_CMB_BTN_TYPE, NULL, NULL);
            SendMessage(hCmbBtnType, CB_ADDSTRING, 0, (LPARAM)"左键");
            SendMessage(hCmbBtnType, CB_ADDSTRING, 0, (LPARAM)"右键");
            SendMessage(hCmbBtnType, CB_SETCURSEL, 0, 0); // 默认选左键

            HWND hLbl4 = CreateWindow("STATIC", "点击模式:", WS_VISIBLE | WS_CHILD, 20, 125, 100, 20, hwnd, NULL, NULL, NULL);
            hCmbActType = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 130, 125, 80, 100, hwnd, (HMENU)ID_CMB_ACT_TYPE, NULL, NULL);
            SendMessage(hCmbActType, CB_ADDSTRING, 0, (LPARAM)"单击");
            SendMessage(hCmbActType, CB_ADDSTRING, 0, (LPARAM)"双击");
            SendMessage(hCmbActType, CB_SETCURSEL, 0, 0); // 默认选单击

            // 3. 快捷键选择区域
            HWND hLbl5 = CreateWindow("STATIC", "开启/暂停热键:", WS_VISIBLE | WS_CHILD, 20, 160, 100, 20, hwnd, NULL, NULL, NULL);
            hCmbHkToggle = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 130, 160, 80, 100, hwnd, (HMENU)ID_CMB_HK_TOGGLE, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkToggle);
            SendMessage(hCmbHkToggle, CB_SETCURSEL, 7, 0); // 默认选 F8 (索引7)

            HWND hLbl6 = CreateWindow("STATIC", "强制停止热键:", WS_VISIBLE | WS_CHILD, 20, 195, 100, 20, hwnd, NULL, NULL, NULL);
            hCmbHkStop = CreateWindow("COMBOBOX", "", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE, 130, 195, 80, 100, hwnd, (HMENU)ID_CMB_HK_STOP, NULL, NULL);
            PopulateHotkeyCombo(hCmbHkStop);
            SendMessage(hCmbHkStop, CB_SETCURSEL, 8, 0); // 默认选 F9 (索引8)

            // 4. 应用按钮与状态栏
            hBtnApply = CreateWindow("BUTTON", "应用所有设置", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 20, 240, 290, 30, hwnd, (HMENU)ID_BTN_APPLY, NULL, NULL);
            hStatusLabel = CreateWindow("STATIC", ">> 状态: 已准备就绪", WS_VISIBLE | WS_CHILD, 20, 285, 300, 20, hwnd, NULL, NULL, NULL);
            
            // 界面美化：应用默认字体
            SetDefaultFont(hLbl1); SetDefaultFont(hEditMin);
            SetDefaultFont(hLbl2); SetDefaultFont(hEditMax);
            SetDefaultFont(hLbl3); SetDefaultFont(hCmbBtnType);
            SetDefaultFont(hLbl4); SetDefaultFont(hCmbActType);
            SetDefaultFont(hLbl5); SetDefaultFont(hCmbHkToggle);
            SetDefaultFont(hLbl6); SetDefaultFont(hCmbHkStop);
            SetDefaultFont(hBtnApply); SetDefaultFont(hStatusLabel);

            // 启动后台工作线程
            CreateThread(NULL, 0, WorkerThread, (LPVOID)hwnd, 0, NULL);
            break;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT); // 设置文本背景为透明
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE); // 返回与窗口一致的画刷
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BTN_APPLY) {
                // 1. 读取并校验时间间隔
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

                // 2. 读取动作类型
                action_button = SendMessage(hCmbBtnType, CB_GETCURSEL, 0, 0); // 0:左, 1:右
                action_mode   = SendMessage(hCmbActType, CB_GETCURSEL, 0, 0); // 0:单, 1:双

                // 3. 读取并映射快捷键 (F1的虚拟键码是0x70, F12是0x7B)
                int toggle_idx = SendMessage(hCmbHkToggle, CB_GETCURSEL, 0, 0);
                int stop_idx   = SendMessage(hCmbHkStop, CB_GETCURSEL, 0, 0);
                
                hotkey_toggle = VK_F1 + toggle_idx;
                hotkey_stop   = VK_F1 + stop_idx;

                MessageBox(hwnd, "参数与快捷键已成功应用！\n(请确保开启和停止不要设置为同一个按键)", "提示", MB_OK | MB_ICONINFORMATION);
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
