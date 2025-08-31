#include "pch.h"
#include "LinkWithRobot.h"
#include "sstream"
#include <memory>
#include <winrt/Windows.Storage.h>

LinkWithRobot* globalLink = nullptr;

void cleanup()
{
    delete globalLink;
}
void LinkWithRobot::sendMesenge(std::wstring input)
{
    std::lock_guard<std::mutex> lock(mtx);
    DWORD bytesWritten = 0;
    input += L"\r\n";
    WriteFile(hTx, input.c_str(), (DWORD)input.size(), &bytesWritten, NULL);
}
void LinkWithRobot::sendMesenge(std::string input)
{
    std::lock_guard<std::mutex> lock(mtx);
    DWORD bytesWritten = 0;
    input += "\r\n";
    WriteFile(hTx, input.c_str(), (DWORD)input.size(), &bytesWritten, NULL);
}
LinkWithRobot::LinkWithRobot(std::string address)
{
    globalLink = this;
    std::atexit(cleanup);
    char buffer[MAX_PATH] = { 0 };
    if (SearchPathA(NULL, "python.exe", NULL, MAX_PATH, buffer, NULL) == 0)
    {
        errorMesenge = L"Python не найден в PATH.";
        return;
    }
    std::string pythonExe(buffer);
    winrt::Windows::Storage::StorageFolder folder = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation();
    std::string fullPath = winrt::to_string(folder.Path()).c_str();
    fullPath += "\\BluetoothDriver.py";

    std::string command = "\"" + pythonExe + "\" \"" + fullPath + "\" \"" + address + "\"";

    STARTUPINFOA si = { 0 };

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    this->address = address;

    hTx = CreateNamedPipeW(
        PIPE_TX,
        PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_WAIT,
        1, 512, 512, 0, NULL
    );

    if (hTx == INVALID_HANDLE_VALUE) {
        errorMesenge = L"Failed to create Tx pipe";
        return;
    }

    hRx = CreateNamedPipeW(
        PIPE_RX,
        PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_WAIT,
        1, 512, 512, 0, NULL
    );

    if (hRx == INVALID_HANDLE_VALUE) {
        errorMesenge = L"Failed to create Rx pipe";
        CloseHandle(hTx);
        return;
    }

    if (!CreateProcessA(
        NULL,             // Программа
        &command[0],      // Командная строка
        NULL,             // Параметры безопасности
        NULL,             // Параметры безопасности
        FALSE,            // Не перенаправляем ручки
        0,                // Флаги создания
        NULL,             // Среда
        NULL,             // Директория
        &si,              // Информация о старте
        &pi               // Информация о процессе
    )) {
        errorMesenge = L"Error creating process: " + GetLastError();
        return;
    }
    Sleep(500);
    ConnectNamedPipe(hTx, NULL);
    ConnectNamedPipe(hRx, NULL);
    shouldTerminate = false;

    std::thread monitorThread(&LinkWithRobot::monitorData, this);
    monitorThread.detach();
}

LinkWithRobot::~LinkWithRobot()
{
    if (pi.hProcess) {
        shouldTerminate = true;
        TerminateProcess(pi.hProcess, 0); 
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}
void  LinkWithRobot::monitorData() {
    char buffer[512];
    DWORD bytesRead = 0;

    std::chrono::steady_clock::time_point lastResponseTime = std::chrono::steady_clock::now();
    bool waitingForResponse = false;
    DWORD bytesAvailable = 0;
    bool isRunProgram = true;
    while (!shouldTerminate) {
        auto now = std::chrono::steady_clock::now();

        if (!waitingForResponse && isConnected &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastResponseTime) >= requestInterval) {
            sendMesenge("GetState();");
            waitingForResponse = true;
        }

        if (waitingForResponse && isConnected &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastResponseTime) > timeout) {
            if (isRunProgram == true)
            {
                fromRobot.push(L"RECEIVED: Sleep();");
                fromRobot.push(L"RECEIVED: SleepRobot();");
            }
            isRunProgram = false;
            waitingForResponse = false;
            lastResponseTime = now;
        }

        DWORD bytesAvailable = 0;
        if (PeekNamedPipe(hRx, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable > 0) {
            if (ReadFile(hRx, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                buffer[bytesRead] = '\0';
                std::wstring wstr(buffer, buffer + bytesRead);
                waitingForResponse = false;
                if (wstr.find(L"State(") != -1 && wstr.find(L"GetState(") == -1)
                {
                    isRunProgram = true;
                    lastResponseTime = now;
                }
                if (wstr.substr(0, 7) == L"ERROR: ") {
                    errorMesenge = wstr.substr(7);
                }
                else {
                    if (wstr.find(L"\n") != -1)
                    {
                        std::wistringstream iss(wstr);
                        std::wstring token;
                        while (std::getline(iss, token, L'\n')) {
                            if (!token.empty()) {
                                fromRobot.push(token + L"\n");
                            }
                        }
                    }
                    else
                    {
                        fromRobot.push(wstr);
                    }
                }
            }
        }

        Sleep(10);
    }
}