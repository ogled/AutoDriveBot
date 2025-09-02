#include "pch.h"
#include "KeyHandler.h"
#include "MainPageApp.xaml.h"
#include <winuser.h>
#include <iostream>

HHOOK KeyHandler::s_hook = nullptr;
winrt::TLTC__winUi3_::implementation::MainPageApp* winrt::TLTC__winUi3_::implementation::MainPageApp::s_instance = nullptr;
KeyHandler* KeyHandler::s_instanceKey = nullptr;

KeyHandler::KeyHandler() {
    s_instanceKey = this;
}

KeyHandler::~KeyHandler() {
    UninstallHook();
    s_instanceKey = nullptr;
}

void KeyHandler::InstallHook() {
    if (!s_hook) {
        DWORD threadId = GetCurrentThreadId();
        s_hook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, nullptr, threadId);
    }
}

void KeyHandler::UninstallHook() {
    if (s_hook) {
        UnhookWindowsHookEx(s_hook);
        s_hook = nullptr;
    }
}

LRESULT CALLBACK KeyHandler::GetMsgProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code >= 0 && winrt::TLTC__winUi3_::implementation::MainPageApp::s_instance->isJoysticMode) {
        MSG* msg = reinterpret_cast<MSG*>(lParam);
        if (msg->wParam == VK_UP) {
            if (msg->message == WM_KEYDOWN) {
                if (!s_instanceKey->m_keyUpPressed) {
                    s_instanceKey->m_keyUpPressed = true;
                    winrt::TLTC__winUi3_::implementation::MainPageApp::s_instance->link->sendMesenge("SM(100);");
                }
            }
            else if (msg->message == WM_KEYUP && s_instanceKey->m_keyUpPressed) {
                s_instanceKey->m_keyUpPressed = false;
                winrt::TLTC__winUi3_::implementation::MainPageApp::s_instance->link->sendMesenge("SM(0);");
            }
        }
        else if (msg->wParam == VK_DOWN) {
            if (msg->message == WM_KEYDOWN) {
                if (!s_instanceKey->m_keyUpPressed) {
                    s_instanceKey->m_keyUpPressed = true;
                    winrt::TLTC__winUi3_::implementation::MainPageApp::s_instance->link->sendMesenge("SM(-100);");
                }
            }
            else if (msg->message == WM_KEYUP && s_instanceKey->m_keyUpPressed) {
                s_instanceKey->m_keyUpPressed = false;
                winrt::TLTC__winUi3_::implementation::MainPageApp::s_instance->link->sendMesenge("SM(0);");
            }
        }
        else if (msg->wParam == VK_LEFT) {
            if (msg->message == WM_KEYDOWN) {
                if (!s_instanceKey->m_keyUpPressed) {
                    s_instanceKey->m_keyUpPressed = true;
                    winrt::TLTC__winUi3_::implementation::MainPageApp::s_instance->link->sendMesenge("ST(-65);");
                }
            }
        }
        else if (msg->wParam == VK_RIGHT) {
            if (msg->message == WM_KEYDOWN) {
                if (!s_instanceKey->m_keyUpPressed) {
                    s_instanceKey->m_keyUpPressed = true;
                    winrt::TLTC__winUi3_::implementation::MainPageApp::s_instance->link->sendMesenge("ST(65);");
                }
            }
        }
    }
    return CallNextHookEx(s_hook, code, wParam, lParam);
}