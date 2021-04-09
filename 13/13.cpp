//https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program

#include <windows.h>

char windowProc;

LRESULT __stdcall Entry(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (windowProc)
    {
        switch (uMsg)
        {
        case WM_DESTROY: PostQuitMessage(0);
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    windowProc = 1;
    static const char hello[]  = "hello";
    WNDCLASS wc = {0,Entry,0,0,0,0,0,(HBRUSH)1,0,hello};
    RegisterClass(&wc);
    ShowWindow(CreateWindowEx(0,hello,hello,WS_OVERLAPPEDWINDOW,0,0,0,0,0,0,0,0), SW_MAXIMIZE);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
        DispatchMessage(&msg);

    return 0;
}
