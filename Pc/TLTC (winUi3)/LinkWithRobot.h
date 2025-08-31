#include <mutex>
#include <queue>
#pragma once
class LinkWithRobot
{
public:
    void LinkWithRobot::sendMesenge(std::wstring input);
    void LinkWithRobot::sendMesenge(std::string input);
    std::wstring errorMesenge = L"";
    bool isConnected = false;
    std::queue<std::wstring> fromRobot{ std::deque<std::wstring>{L"RECEIVED: Sleep();"} };
    std::chrono::milliseconds requestInterval{ 1000 };
    LinkWithRobot() {};
	LinkWithRobot(std::string address);
    ~LinkWithRobot();
private:
    std::chrono::milliseconds timeout{ 3000 };

    std::mutex mtx;
    std::atomic<bool> shouldTerminate;
    void  LinkWithRobot::monitorData();
    HANDLE hTx;
    HANDLE hRx;
    PROCESS_INFORMATION pi = { 0 };
    const wchar_t* PIPE_TX = L"\\\\.\\pipe\\AutoDriveBotTx";
    const wchar_t* PIPE_RX = L"\\\\.\\pipe\\AutoDriveBotRx";
    std::string address;
    std::string RunCommand(const std::string& command)
    {
        std::array<char, 128> buffer;
        std::string result;
        std::shared_ptr<FILE> pipe(_popen(command.c_str(), "r"), _pclose);
        if (!pipe) throw std::runtime_error("Ошибка при запуске команды");

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }
};

