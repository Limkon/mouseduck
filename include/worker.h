#ifndef WORKER_H
#define WORKER_H

#include <windows.h>

// 线程入口函数声明
DWORD WINAPI WorkerThread(LPVOID lpParam);

#endif // WORKER_H
