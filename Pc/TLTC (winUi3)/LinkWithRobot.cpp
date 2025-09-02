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
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 512, 512, 0, NULL
    );

    if (hTx == INVALID_HANDLE_VALUE) {
        errorMesenge = L"Failed to create Tx pipe";
        return;
    }

    hRx = CreateNamedPipeW(
        PIPE_RX,
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
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
    hJob = CreateJobObject(NULL, NULL);

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
    jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));

    AssignProcessToJobObject(hJob, pi.hProcess);
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
    DWORD bytesAvailable = 0;
    bool isRunProgram = true;
    bool isWaitMs = false;
    while (!shouldTerminate) {
        auto now = std::chrono::steady_clock::now();

        if (!isWaitMs && isConnected &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastResponseTime) > lowtimeout && isRunProgram) {
            sendMesenge("MS();");
            isWaitMs = true;
        }
        else if (isConnected &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - lastResponseTime) > hightimeout && isRunProgram)
        {
            fromRobot.push(L"RECEIVED: Sleep();");
            fromRobot.push(L"RECEIVED: SleepRobot();");
            isRunProgram = false;
            isWaitMs = false;
        }

        DWORD bytesAvailable = 0;
        if (PeekNamedPipe(hRx, NULL, 0, NULL, &bytesAvailable, NULL) && bytesAvailable > 0) {
            isWaitMs = false;
            while (bytesAvailable > 0) {
                if (ReadFile(hRx, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    std::wstring wstr(buffer, buffer + bytesRead);

                    if (wstr.rfind(L"ERROR: ", 0) == 0) {
                        errorMesenge = wstr.substr(7);
                    }
                    else if(!wstr.empty()){
                        isRunProgram = true;
                        lastResponseTime = now;
                        std::wistringstream iss(wstr);
                        std::wstring token;
                        while (std::getline(iss, token, L'\n')) {
                            if (!token.empty()) {
                                fromRobot.push(token + L"\n");
                            }
                        }
                    }
                }
                PeekNamedPipe(hRx, NULL, 0, NULL, &bytesAvailable, NULL);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}