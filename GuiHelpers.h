#ifndef GUI_H
#define GUI_H

#include <Windows.h>
#include <CommCtrl.h>

#include <string>

namespace gui
{
    namespace helpers
    {
        using namespace std;

        struct Edit
        {
            static wstring GetText(HWND editControl);
            static int GetInt(HWND edit);
            static void SetInt(HWND edit, int value);
        };

        struct UpDown
        {
            static int GetValue(HWND upDown);
        };
    }
}

#endif
