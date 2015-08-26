#include "Engine.h"

namespace spamer
{
    HWND Engine::skypeWnd = NULL;
    Engine::BoolEx Engine::useCtrlEnter = Unknown;

    bool Engine::IsSkypeRunning()
    {
        if (skypeWnd == NULL)
        {
            skypeWnd = FindWindow(L"tSkMainForm", nullptr);
        }
        return (skypeWnd != NULL);
    }

    HWND Engine::GetChatHandle(HWND conversation)
    {
        HWND chatEntry = FindWindowEx(conversation, 0, L"TChatEntryControl", nullptr);
        return FindWindowEx(chatEntry, 0, L"TChatRichEdit", nullptr);
    }

    vector<Conversation> Engine::GetConversations()
    {
        vector<Conversation> result;
        if (IsSkypeRunning())
        {
            EnumChildWindows(skypeWnd, static_cast<WNDENUMPROC>(&EnumWindowProc), reinterpret_cast<LPARAM>(&result));
        }
        return result;
    }

    WINBOOL CALLBACK Engine::EnumWindowProc(HWND childWnd, LPARAM lParam)
    {
        TCHAR wndClass[256] = { 0 };

        if (GetClassName(childWnd, wndClass, 255) &&
            wcscmp(wndClass, L"TConversationForm") == 0)
        {
            auto len = GetWindowTextLength(childWnd);
            auto buffer = new TCHAR[len + 1];

            GetWindowText(childWnd, buffer, len + 1);

            auto conversations = reinterpret_cast<vector<Conversation>*>(lParam);
            conversations->emplace_back(wstring(buffer), GetChatHandle(childWnd));

            delete[] buffer;
        }
        return TRUE;
    }

    void Engine::PrepareSending()
    {
        auto skypeThreadID = GetWindowThreadProcessId(skypeWnd, nullptr);
        auto thisThreadID = GetCurrentThreadId();

        AttachThreadInput(thisThreadID, skypeThreadID, true);
    }

    void Engine::FinishSending()
    {
        auto skypeThreadID = GetWindowThreadProcessId(skypeWnd, nullptr);
        auto thisThreadID = GetCurrentThreadId();

        AttachThreadInput(thisThreadID, skypeThreadID, false);
    }

    void Engine::PressCtrlKey()
    {
        static BYTE keyStates[256];
        GetKeyboardState(keyStates);
        keyStates[VK_CONTROL] = 0xFF;
        SetKeyboardState(keyStates);
    }

    void Engine::PressEnter(HWND chatWindow)
    {
        ::SendMessage(chatWindow, WM_KEYDOWN, VK_RETURN, 0);
        ::SendMessage(chatWindow, WM_KEYUP, VK_RETURN, 0);
    }

    void Engine::SendMessage(const wstring& message, const Conversation& conv)
    {
        ::SendMessage(conv.chatWindow, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(message.c_str()));

        switch (useCtrlEnter)
        {
        case No:
            PressEnter(conv.chatWindow);
            break;

        case Unknown:
            PressEnter(conv.chatWindow);
            if (::SendMessage(conv.chatWindow, WM_GETTEXTLENGTH, 0, 0) == 0)
            {
                useCtrlEnter = No;
                break;
            }
            useCtrlEnter = Yes;

        case Yes:
            PressCtrlKey();
            PressEnter(conv.chatWindow);
            break;
        }
    }

    void Engine::SendSpam(const SendingInfo& info)
    {
        PrepareSending();
        for (int k = 1; k <= info.count; k++)
        {
            Engine::SendMessage(info.message, info.conversation);
            if (info.progress)
                info.progress->store(k);
        }
        FinishSending();
    }
}
