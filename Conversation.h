#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <windows.h>

#include <string>

namespace spamer
{
    using namespace std;

    class Conversation
    {
        friend class Engine;

    public:
        Conversation() :
            title(L""), chatWindow(NULL) {}
        Conversation(const wstring& _title, HWND _chat) :
            title(_title), chatWindow(_chat) {}

        wstring GetTitle() const
        {
            return title;
        }

    private:
        wstring title;
        HWND chatWindow;
    };
}

#endif
