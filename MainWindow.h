#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Engine.h"

#include <windows.h>
#include <commctrl.h>

#include <vector>
#include <string>
#include <atomic>
#include <chrono>

namespace gui
{
    using namespace std;
    using namespace spamer;
    using namespace std::chrono;

    class MainWindow
    {
    public:
        static MainWindow& GetInstance();

        bool IsCreated() const;

    private:
        static const int MaxSpamMessages = 1024;

        static const int ProgressTimerID = 444;
        static const int ProgressTimerPeriod = 400;

        static const int Width = 380;
        static const int Height = 285;

        static LPCWSTR constexpr Class = L"SkySpamerWndClass";
        static LPCWSTR constexpr Title = L"SkySpamka";
        static LPCWSTR constexpr AboutMessage = L"  SkySpamka\n\n  Quite a nice thang\n  Supports Skype 5.5+, Windows XP and later\n\n  Author: ToLik Boiko";

        static MainWindow* instance;

        static bool CheckVirtualKey(WPARAM vk);

        static DWORD WINAPI WorkerThreadProc(void* param);
        static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK ComboBoxWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK AmountEditWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        MainWindow(const MainWindow& value) = delete;
        MainWindow(MainWindow&& rvalue) = delete;
        MainWindow operator=(const MainWindow& value) = delete;
        MainWindow operator=(MainWindow&& rvalue) = delete;

        MainWindow(HINSTANCE application);

        bool RegisterWindowClass() const;
        bool CreateMainWindow();

        void CreateFonts();
        void CreateControls(HWND parent);

        void UpdateConversations();
        void EnableControls(bool enable);

        void SetupProgressTimer();
        void DestroyProgressTimer();

        void StartSpam(const Conversation& conv);
        void RunWorkerThread(const Conversation& conv);
        void StopSpam();
        void FinishSpam();
        void ShowResults(high_resolution_clock::duration timeElapsed) const;

        void StartButtonClick();
        void ShowAboutBox();

        int prevSpamCount = 4;
        int messageCount;

        HWND hWnd;
        HINSTANCE hInstance;
        HANDLE workerThread;

        atomic<int> progress;
        vector<Conversation> conversations;
        high_resolution_clock::time_point startTime;

        HFONT verdanaFont;
        HFONT sylfaenFont;

        HWND btAbout;
        HWND btSendSpam;
        HWND btStopSpam;

        HWND cbConversations;
        HWND stConversations;

        HWND edSpamMessage;
        HWND stSpamMessage;
        HWND progressBar;

        HWND stSpamCount;
        HWND udSpamCount;
        HWND edSpamCount;

        LONG comboboxOriginalWndProc;
        LONG amountOriginalWndProc;
    };
}

#endif
