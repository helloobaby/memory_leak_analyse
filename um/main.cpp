#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // 减少 Windows.h 的污染
#endif

#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <iostream>
#include <stdio.h>

// 必须链接这两个库
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")

// 如果还报错，手动定义一下这个宏
#ifndef AF_UNSPEC
#define AF_UNSPEC 0
#endif

// 1. 堆内存泄漏：API 分配了内部缓冲区要求调用者释放
void Leak_ApiInternalBuffer() {
    DWORD dwSize = 0;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;

    // 第一次调用获取大小
    GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &dwSize);

    // 这种 API 通常使用 HeapAlloc 分配在进程堆上
    pAddresses = (IP_ADAPTER_ADDRESSES*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);

    if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pAddresses, &dwSize) == NO_ERROR) {
        // 业务逻辑...
    }
    // 【泄漏点】：应该调用 HeapFree(GetProcessHeap(), 0, pAddresses);
}

// 2. 内核对象泄漏：句柄未关闭 (Handle Leak)
// 这会导致内核内存（Non-paged Pool）持续增长，!ust 对此无效，需用 !htrace
void Leak_KernelHandle() {
    // 每次调用都打开当前进程的句柄而不关闭
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
    if (hProcess) {
        // 使用句柄做点事...
        DWORD priority = GetPriorityClass(hProcess);
    }
    // 【泄漏点】：缺失 CloseHandle(hProcess);
}

// 3. GDI 对象泄漏：DC 与位图
// 这会导致 GDI 句柄数达到 10000 的硬上限后导致 UI 崩溃
void Leak_GdiObject() {
    HDC hdc = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, 100, 100);
    SelectObject(memDC, hBitmap);

    // 【泄漏点】：
    // 1. 应该先 SelectObject(memDC, oldBitmap); 还原
    // 2. 应该 DeleteObject(hBitmap);
    // 3. 应该 DeleteDC(memDC);
    // 4. 应该 ReleaseDC(NULL, hdc);
}

int main() {
    printf("Hardcore Leak Demo Running...\n");
    while (true) {
        Leak_ApiInternalBuffer(); // 堆内存泄漏 (UST 覆盖范围)
        //Leak_KernelHandle();      // 句柄泄漏 (内核追踪范围)
        //Leak_GdiObject();         // GDI 资源泄漏
        Sleep(50);
    }
    return 0;
}