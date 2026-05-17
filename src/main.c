#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// 引入自定义头文件
#include "globals.h"
#include "worker.h"
#include "resource.h"

// ====== 启用 Windows 现代视觉样式 (让按钮和输入框具有系统原生现代化外观) ======
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
// ==============================================================================

// 实例化全局变量（在 globals.h 中仅作了 extern 声明）
bool is_active = false;
int interval_min = 30;
int interval_max = 30;
HWND hStatusLabel = NULL;

// 局部 UI 控件句柄
HWND hEditMin, hEditMax, hBtnApply;

// 窗口过程函数声明
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 程序入口点 (替代 main，可隐藏控制台黑框)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "SysToolWindowClass";
    
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    
    // ====== 修复GUI色差(1/2)：将窗口底色改为系统标准控件面板的颜色 ======
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); 
    // =====================================================================
    
    // 加载图标 (引用 resource.h 中的宏)
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc)) {
        return 0;
    }

    // 创建主窗口 (固定大小，不可最大化)
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, "SysTool", 
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 220,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// 设置控件使用系统默认字体
void SetDefaultFont(HWND hwnd) {
    SendMessage(hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
}

// 窗口消息处理过程
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // 创建静态文本和输入框
            HWND hLbl1 = CreateWindow("STATIC", "最小间隔 (ms):", WS_VISIBLE | WS_CHILD, 20, 20, 100, 20, hwnd, NULL, NULL, NULL);
            hEditMin = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 130, 20, 80, 20, hwnd, (HMENU)ID_EDIT_MIN, NULL, NULL);
            
            HWND hLbl2 = CreateWindow("STATIC", "最大间隔 (ms):", WS_VISIBLE | WS_CHILD, 20, 55, 100, 20, hwnd, NULL, NULL, NULL);
            hEditMax = CreateWindow("EDIT", "30", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 130, 55, 80, 20, hwnd, (HMENU)ID_EDIT_MAX, NULL, NULL);

            // 创建应用按钮
            hBtnApply = CreateWindow("BUTTON", "应用设置", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 230, 20, 80, 55, hwnd, (HMENU)ID_BTN_APPLY, NULL, NULL);

            // 创建状态提示标签
            hStatusLabel = CreateWindow("STATIC", ">> 状态: 已暂停 [F8开启/F9退出]", WS_VISIBLE | WS_CHILD, 20, 100, 300, 20, hwnd, NULL, NULL, NULL);
            
            // 界面美化：应用默认字体
            SetDefaultFont(hLbl1); SetDefaultFont(hEditMin);
            SetDefaultFont(hLbl2); SetDefaultFont(hEditMax);
            SetDefaultFont(hBtnApply); SetDefaultFont(hStatusLabel);

            // 启动后台工作线程 (将主窗口句柄作为参数传入，以便线程能发送退出消息)
            CreateThread(NULL, 0, WorkerThread, (LPVOID)hwnd, 0, NULL);
            break;
        }

        // ====== 修复GUI色差(2/2)：拦截静态控件绘制，使其背景透明并融合窗口底色 ======
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT); // 设置文本背景为透明
            return (LRESULT)GetSysColorBrush(COLOR_BTNFACE); // 返回与窗口一致的画刷
        }
        // ==============================================================================

        case WM_COMMAND: {
            // 捕获按钮点击事件
            if (LOWORD(wParam) == ID_BTN_APPLY) {
                char szMin[16], szMax[16];
                GetWindowText(hEditMin, szMin, 16);
                GetWindowText(hEditMax, szMax, 16);

                int temp_min = atoi(szMin);
                int temp_max = atoi(szMax);

                // 简单的合法性校验：最小不能低于1ms，最大不能小于最小
                if (temp_min < 1) temp_min = 1;
                if (temp_max < temp_min) temp_max = temp_min;

                // 更新文本框显示纠正后的值
                sprintf(szMin, "%d", temp_min);
                sprintf(szMax, "%d", temp_max);
                SetWindowText(hEditMin, szMin);
                SetWindowText(hEditMax, szMax);

                // 安全地写入全局变量
                interval_min = temp_min;
                interval_max = temp_max;

                MessageBox(hwnd, "执行参数已更新！", "提示", MB_OK | MB_ICONINFORMATION);
            }
            break;
        }

        case WM_CLOSE:
            // 拦截关闭请求，销毁窗口
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
