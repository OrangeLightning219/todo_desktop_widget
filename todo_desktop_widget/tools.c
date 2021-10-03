#include "tools.h"
#include "windows.h"

void setWindowOnBottom(void* handle)
{
    // SetParent((HWND)handle, (HWND) FindWindow("Progman", "Program Manager"));
    SetWindowPos((HWND)handle, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

}

void removeWindowFromTaskbar(void *handle)
{

}