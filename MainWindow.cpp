#include "MainWindow.h"
#include "GuiHelpers.h"

#include <string>
#include <sstream>
#include <cstdlib>
#include <chrono>
#include <memory>

namespace gui
{
    using namespace std;
    using namespace std::chrono;
    using namespace gui::helpers;

    MainWindow* MainWindow::instance = nullptr;

    MainWindow::MainWindow(HINSTANCE application) :
        hInstance(application)
    {
        MainWindow::instance = this;

        InitCommonControls();
        RegisterWindowClass() && CreateMainWindow();
    }

    MainWindow& MainWindow::GetInstance()
    {
        static MainWindow instance(GetModuleHandle(nullptr));
        return instance;
    }

    bool MainWindow::IsCreated() const
    {
        return (hWnd != NULL);
    }

    bool MainWindow::RegisterWindowClass() const
    {
        WNDCLASSEX wcex;

        RtlZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);

        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = static_cast<WNDPROC>(MainWindow::MainWndProc);
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = NULL;
        wcex.hCursor = LoadCursor(0, IDC_ARROW);
        wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1);
        wcex.lpszClassName = MainWindow::Class;
        wcex.hIconSm = LoadIcon(0, IDI_APPLICATION);

        return RegisterClassEx(&wcex) != 0;
    }

    bool MainWindow::CreateMainWindow()
    {
        auto x = (GetSystemMetrics(SM_CXSCREEN) - MainWindow::Width) / 2;
        auto y = (GetSystemMetrics(SM_CYSCREEN) - MainWindow::Height) / 2;

        if ((hWnd = CreateWindow(MainWindow::Class, MainWindow::Title,
                 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                 x, y, MainWindow::Width, MainWindow::Height, NULL, NULL, hInstance, NULL)))
        {

            ShowWindow(hWnd, SW_SHOW);
            return true;
        }
        return false;
    }

    void MainWindow::ShowAboutBox()
    {
        MessageBox(hWnd, MainWindow::AboutMessage, L"About", MB_ICONINFORMATION);
    }

    void MainWindow::UpdateConversations()
    {
        conversations = Engine::GetConversations();
        SendMessage(cbConversations, CB_RESETCONTENT, 0, 0);

        for (auto conv : conversations)
        {
            SendMessage(cbConversations, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(conv.GetTitle().c_str()));
        }

        if (!conversations.empty())
        {
            SendMessage(cbConversations, CB_SETCURSEL, 0, 0);
        }
    }

    void MainWindow::EnableControls(bool enable)
    {
        EnableWindow(cbConversations, enable);
        EnableWindow(edSpamMessage, enable);
        EnableWindow(edSpamCount, enable);
        EnableWindow(udSpamCount, enable);
        EnableWindow(btSendSpam, enable);

        EnableWindow(btStopSpam, !enable);
    }

    void MainWindow::SetupProgressTimer()
    {
        SetTimer(hWnd, MainWindow::ProgressTimerID, MainWindow::ProgressTimerPeriod, nullptr);
    }

    void MainWindow::DestroyProgressTimer()
    {
        KillTimer(hWnd, MainWindow::ProgressTimerID);
    }

    void MainWindow::StartButtonClick()
    {
        if (conversations.empty())
        {
            MessageBox(hWnd, L"Start a conversation", L"Umm...", MB_ICONINFORMATION | MB_OK);
            return;
        }

        auto selection = SendMessage(cbConversations, CB_GETCURSEL, 0, 0);
        StartSpam(conversations[selection]);
    }

    void MainWindow::StartSpam(const Conversation& conv)
    {
        startTime = high_resolution_clock::now();
        messageCount = UpDown::GetValue(udSpamCount);

        EnableControls(false);

        SendMessage(progressBar, PBM_SETPOS, 0, 0);
        SendMessage(progressBar, PBM_SETRANGE, 0, MAKELPARAM(0, messageCount));

        progress.store(0);

        SetupProgressTimer();
        RunWorkerThread(conv);
    }

    void MainWindow::RunWorkerThread(const Conversation& conv)
    {
        static SendingInfo info;

        info.conversation = conv;
        info.progress = &progress;
        info.count = messageCount;
        info.message = Edit::GetText(edSpamMessage);

        workerThread = CreateThread(0, 0, static_cast<LPTHREAD_START_ROUTINE>(&WorkerThreadProc), &info, 0, nullptr);
    }

    DWORD WINAPI MainWindow::WorkerThreadProc(void* param)
    {
        Engine::SendSpam(*reinterpret_cast<SendingInfo*>(param));
        return 0;
    }

    void MainWindow::StopSpam()
    {
        TerminateThread(workerThread, 0);
        FinishSpam();
    }

    void MainWindow::FinishSpam()
    {
        CloseHandle(workerThread);
        DestroyProgressTimer();

        ShowResults(high_resolution_clock::now() - startTime);
        EnableControls(true);
        SetWindowText(hWnd, MainWindow::Title);
    }

    void MainWindow::ShowResults(high_resolution_clock::duration timeElapsed) const
    {
        auto h = duration_cast<hours>(timeElapsed);
        auto m = duration_cast<minutes>(timeElapsed -= h);
        auto s = duration_cast<seconds>(timeElapsed -= m);
        auto ms = duration_cast<milliseconds>(timeElapsed -= s);

        wostringstream msg;
        msg << L"Time elapsed:     "
            << h.count() << " : "
            << m.count() << " : "
            << s.count() << "."
            << ms.count() << "\n"

            << L"Messages sent:   "
            << progress.load();

        MessageBox(hWnd, msg.str().c_str(), L"It's over", MB_OK);
    }

    void MainWindow::CreateFonts()
    {
        verdanaFont = CreateFont(-13, 0, 0, 0, 400, FALSE, FALSE, FALSE, 1, 400, 0, 0, 0, L"Verdana");
        sylfaenFont = CreateFont(-15, 0, 0, 0, 400, FALSE, FALSE, FALSE, 1, 400, 0, 0, 0, L"Sylfaen");
    }

    void MainWindow::CreateControls(HWND parent)
    {
        // Conversations combobox
        cbConversations = CreateWindow(WC_COMBOBOX, NULL,
            WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER | CBS_DROPDOWNLIST,
            12, 30, 350, 24, parent, 0, hInstance, 0);
        comboboxOriginalWndProc = SetWindowLong(cbConversations, GWL_WNDPROC, reinterpret_cast<LONG>(ComboBoxWndProc));
        SendMessage(cbConversations, WM_SETFONT, reinterpret_cast<WPARAM>(verdanaFont), FALSE);

        // Conversations label
        stConversations = CreateWindow(WC_STATIC, L"Active conversations",
            WS_VISIBLE | WS_CHILD, 12, 5, 141, 24, parent, 0, hInstance, 0);
        SendMessage(stConversations, WM_SETFONT, reinterpret_cast<WPARAM>(sylfaenFont), FALSE);

        // Spam message multiline field
        edSpamMessage = CreateWindowEx(WS_EX_CLIENTEDGE, L"edit", L"Hello there!",
            WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | WS_TABSTOP | ES_MULTILINE,
            12, 86, 201, 126, parent, 0, hInstance, 0);
        SendMessage(edSpamMessage, WM_SETFONT, reinterpret_cast<WPARAM>(verdanaFont), FALSE);

        // Spam message label
        stSpamMessage = CreateWindow(WC_STATIC, L"Important message",
            WS_VISIBLE | WS_CHILD, 12, 60, 191, 24, parent, 0, hInstance, 0);
        SendMessage(stSpamMessage, WM_SETFONT, reinterpret_cast<WPARAM>(sylfaenFont), FALSE);

        // Send spam button
        btSendSpam = CreateWindow(WC_BUTTON, L"Ca-boooom",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 221, 142, 105, 32, parent, 0, hInstance, 0);
        SendMessage(btSendSpam, WM_SETFONT, reinterpret_cast<WPARAM>(sylfaenFont), FALSE);

        // About (?) button
        btAbout = CreateWindow(WC_BUTTON, L"?",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 331, 180, 32, 32, parent, 0, hInstance, 0);
        SendMessage(btSendSpam, WM_SETFONT, reinterpret_cast<WPARAM>(sylfaenFont), FALSE);

        // Stop button
        btStopSpam = CreateWindow(WC_BUTTON, L"Enough",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED, 221, 180, 105, 32, parent, 0, hInstance, 0);
        SendMessage(btStopSpam, WM_SETFONT, reinterpret_cast<WPARAM>(sylfaenFont), FALSE);

        wostringstream oss;
        oss << L"Amount (max" << MainWindow::MaxSpamMessages << ")";

        // UpDown edit label
        stSpamCount = CreateWindow(WC_STATIC, oss.str().c_str(),
            WS_VISIBLE | WS_CHILD, 222, 83, 120, 24, parent, 0, hInstance, 0);
        SendMessage(stSpamCount, WM_SETFONT, (WPARAM)sylfaenFont, FALSE);

        // UpDown edit field
        edSpamCount = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, L"4",
            WS_VISIBLE | WS_CHILD, 221, 108, 105, 24, parent, 0, hInstance, 0);
        amountOriginalWndProc = SetWindowLong(edSpamCount, GWL_WNDPROC, reinterpret_cast<LONG>(AmountEditWndProc));
        SendMessage(edSpamCount, WM_SETFONT, reinterpret_cast<WPARAM>(verdanaFont), FALSE);

        // UpDown control
        udSpamCount = CreateWindowEx(WS_EX_LTRREADING, UPDOWN_CLASS, NULL,
            WS_CHILDWINDOW | WS_VISIBLE | UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
            0, 0, 0, 0, parent, NULL, hInstance, NULL);
        SendMessage(udSpamCount, UDM_SETRANGE, 0, MAKELPARAM(1, MainWindow::MaxSpamMessages));
        SendMessage(udSpamCount, UDM_SETPOS, 0, 4);

        // Progress bar
        progressBar = CreateWindowEx(WS_EX_CLIENTEDGE, PROGRESS_CLASS, nullptr,
            WS_VISIBLE | WS_CHILD | PBS_SMOOTH, 12, 222, 350, 24, parent, NULL, hInstance, NULL);
    }

    LRESULT CALLBACK MainWindow::MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
            instance->CreateFonts();
            instance->CreateControls(hWnd);
            instance->UpdateConversations();
            break;

        case WM_TIMER:
        {
            auto progress = instance->progress.load();
            auto percentage = progress * 100 / instance->messageCount;

            wostringstream title;
            title << "SkySpamer - " << percentage << "%";

            SetWindowText(hWnd, title.str().c_str());
            SendMessage(instance->progressBar, PBM_SETPOS, progress, 0);

            if (progress == instance->messageCount)
            {
                instance->FinishSpam();
            }
        }
            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                if (lParam == (LPARAM)instance->btAbout)
                    instance->ShowAboutBox();

                if (lParam == (LPARAM)instance->btSendSpam)
                    instance->StartButtonClick();

                if (lParam == (LPARAM)instance->btStopSpam)
                    instance->StopSpam();
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }

    LRESULT CALLBACK MainWindow::ComboBoxWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_LBUTTONDOWN)
        {
            instance->UpdateConversations();
        }
        return CallWindowProc(reinterpret_cast<WNDPROC>(instance->comboboxOriginalWndProc), hWnd, message, wParam, lParam);
    }

    LRESULT CALLBACK MainWindow::AmountEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_CHAR)
        {
            if (!MainWindow::CheckVirtualKey(wParam))
                return 0;

            auto newChar = static_cast<wchar_t>(wParam);
            auto editText = Edit::GetText(hWnd);
            auto originalTextLen = editText.size();

            // Get caret position/text selection
            DWORD begin, end;
            SendMessage(hWnd, EM_GETSEL, reinterpret_cast<WPARAM>(&begin), reinterpret_cast<LPARAM>(&end));

            // Backspace key
            if (newChar == VK_BACK)
            {
                editText.replace(begin, end - begin, L"");
                if (editText.empty() || (end == 1 && originalTextLen == 1))
                {
                    SetWindowText(hWnd, L"1");
                    SendMessage(hWnd, EM_SETSEL, 0, 1);
                    return 0;
                }
            }

            // '0'..'9'
            else
            {
                if (begin == end)
                    editText.insert(begin, 1, newChar);
                else
                    editText.replace(begin, end - begin, wstring(1, newChar));

                auto newValue = wcstol(editText.c_str(), nullptr, 10);
                if (newValue > MaxSpamMessages || newValue < 1)
                {
                    SendMessage(hWnd, EM_SETSEL, 0, originalTextLen);
                    return 0;
                }
            }
        }
        return CallWindowProc(reinterpret_cast<WNDPROC>(instance->amountOriginalWndProc), hWnd, message, wParam, lParam);
    }

    bool MainWindow::CheckVirtualKey(WPARAM vk)
    {
        return (vk == VK_BACK) || (vk >= '0' && vk <= '9');
    }
}
