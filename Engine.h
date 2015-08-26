#ifndef ENGINE_H
#define ENGINE_H

#include "Conversation.h"
#include "SendingInfo.h"

#include <windows.h>

#include <string>
#include <vector>

namespace spamer
{
    using namespace std;

    class Engine
    {
    public:        
        static bool IsSkypeRunning();
        static vector<Conversation> GetConversations();

        static void SendSpam(const SendingInfo& info);

    private:
        enum BoolEx { Yes, No, Unknown };

        Engine();

        static HWND skypeWnd;
        static BoolEx useCtrlEnter;

        static HWND GetChatHandle(HWND conversation);

        static void PressCtrlKey();
        static void PressEnter(HWND chatWindow);

        static void PrepareSending();
        static void FinishSending();
        static void SendMessage(const wstring& message, const Conversation& conv);

        static WINBOOL CALLBACK EnumWindowProc(HWND childWnd, LPARAM lParam);
    };
}

#endif
