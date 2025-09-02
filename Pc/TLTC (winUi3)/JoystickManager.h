#pragma once
#include <pch.h>
#include <winrt/Windows.Gaming.Input.h>
#include <vector>
#include <string>

struct JoystickState {
    std::vector<bool> buttons;
    std::vector<int> axes;
};

class JoystickManager {
public:
    JoystickManager();

    bool Initialize();
    bool isInitialize = false;

    std::vector<std::string> Update();
    
private:
    winrt::Windows::Gaming::Input::Gamepad m_pad{ nullptr };
    JoystickState m_prevState;

    JoystickState GetState();
};
