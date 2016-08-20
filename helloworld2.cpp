#pragma runtime_checks("", off)
#pragma comment(linker, "/nodefaultlib /subsystem:windows")

#include <windows.h>

void WINAPI WinMainCRTStartup()
{
    MessageBoxA(nullptr, "hello world!", "hello", MB_OK);
}
