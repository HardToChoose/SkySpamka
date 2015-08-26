#ifndef SENDINGINFO
#define SENDINGINFO

#include "Conversation.h"

#include <windows.h>

#include <string>
#include <atomic>

namespace spamer
{
    using namespace std;

    struct SendingInfo
    {
        Conversation conversation;
        wstring message;
        int count;
        atomic<int>* progress;
    };
}

#endif
