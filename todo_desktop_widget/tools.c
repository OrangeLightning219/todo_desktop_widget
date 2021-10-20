#include "tools.h"
#include "windows.h"

void SetWindowOnBottom(void *handle)
{
    // SetParent((HWND)handle, (HWND) FindWindow("Progman", "Program Manager"));
    SetWindowPos((HWND)handle, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

}

void RemoveWindowFromTaskbar(void *handle)
{

}

void RemoveConsole()
{
    FreeConsole();
}