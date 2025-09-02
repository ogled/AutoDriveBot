#include <pch.h>

#include "JoystickManager.h"
#include <winrt/Windows.Gaming.Input.h>


using namespace winrt;
using namespace Windows::Gaming::Input;

JoystickManager::JoystickManager()
    : m_prevState{ {false, false, false, false, false, false}, {128, 128, 128, 128, 0, 0} } // 4 кнопки + 4 стика + 2 триггера
{
}

bool JoystickManager::Initialize() {
    if (!isInitialize)
    {
        auto pads = Gamepad::Gamepads();
        if (pads.Size() > 0) {
            m_pad = pads.GetAt(0);
            isInitialize = true;
            return true;
        }
    }
    return false;
}

JoystickState JoystickManager::GetState() {
    auto reading = m_pad.GetCurrentReading();
    JoystickState state;

    // 4 кнопки
    state.buttons.resize(6);
    state.buttons[0] = (reading.Buttons & GamepadButtons::A) == GamepadButtons::A;
    state.buttons[1] = (reading.Buttons & GamepadButtons::B) == GamepadButtons::B;
    state.buttons[2] = (reading.Buttons & GamepadButtons::X) == GamepadButtons::X;
    state.buttons[3] = (reading.Buttons & GamepadButtons::Y) == GamepadButtons::Y;
    state.buttons[4] = (reading.Buttons & GamepadButtons::LeftShoulder) == GamepadButtons::LeftShoulder;
    state.buttons[5] = (reading.Buttons & GamepadButtons::RightShoulder) == GamepadButtons::RightShoulder;

    // Стики
    state.axes.push_back(int((reading.LeftThumbstickX + 1) * 127.5));
    state.axes.push_back(int((reading.LeftThumbstickY + 1) * 127.5));
    state.axes.push_back(int((reading.RightThumbstickX + 1) * 127.5));
    state.axes.push_back(int((reading.RightThumbstickY + 1) * 127.5));

    // Триггеры
    state.axes.push_back(int(reading.LeftTrigger * 255));
    state.axes.push_back(int(reading.RightTrigger * 255));

    return state;
}

std::vector<std::string> JoystickManager::Update() {
    std::vector<std::string> changes;
    if (!m_pad) return changes;

    auto current = GetState();

    // Кнопки
    for (size_t i = 0; i < current.buttons.size(); i++) {
        if (current.buttons[i] != m_prevState.buttons[i]) {
            changes.push_back("T:b" + std::to_string(i) + "," + (current.buttons[i] ? "1" : "0") + "\n");
            m_prevState.buttons[i] = current.buttons[i];
        }
    }

    // Оси
    for (size_t i = 0; i < current.axes.size(); i++) {
        if (current.axes[i] != m_prevState.axes[i]) {
            changes.push_back("T:a" + std::to_string(i) + "," + std::to_string(current.axes[i]) + "\n");
            m_prevState.axes[i] = current.axes[i];
        }
    }

    return changes;
}
