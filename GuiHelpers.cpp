#include "GuiHelpers.h"

#include <memory>

namespace gui
{
    namespace helpers
    {
        wstring Edit::GetText(HWND editControl)
        {
            auto len = GetWindowTextLength(editControl);
            unique_ptr<TCHAR> buffer(new TCHAR[len + 1]);

            GetWindowText(editControl, buffer.get(), len + 1);
            return wstring(buffer.get());
        }

        int Edit::GetInt(HWND edit)
        {
            return wcstol(Edit::GetText(edit).c_str(), nullptr, 10);
        }

        void Edit::SetInt(HWND edit, int value)
        {
            TCHAR buf[16];
            wsprintf(buf, L"%l", value);
            SendMessage(edit, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(buf));
        }

        int UpDown::GetValue(HWND upDown)
        {
            return SendMessage(upDown, UDM_GETPOS32, 0, 0);
        }
    }
}
