#pragma once

#include <windows.h>

class KeyHandler {
public:
    KeyHandler();
    ~KeyHandler();

    void InstallHook();
    void UninstallHook();

    static LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam);

private:
    static HHOOK s_hook;
    static KeyHandler* s_instanceKey;

    bool m_keyUpPressed = false;
};