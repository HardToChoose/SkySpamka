#include <windows.h>

#include "MainWindow.h"
#include "Engine.h"

int main()
{
    if (gui::MainWindow::GetInstance().IsCreated())
    {
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}
