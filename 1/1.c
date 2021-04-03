#include <windows.h>
void Entry()
{
    DWORD a;
    WriteFile((HANDLE)(ptrdiff_t)STD_OUTPUT_HANDLE, "hello\n", 6, &a, 0);
    ExitProcess(0);
}
