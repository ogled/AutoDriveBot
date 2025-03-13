#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <Windows.h>
#include <tinyxml2.h>
#include "Bluetooth.h"
#include <optional>
#include <coroutine>
#include <stringapiset.h>

#pragma once

namespace Helper
{
    static std::string getText(const std::string& tag, const std::string& lang = "ru") {
        tinyxml2::XMLDocument doc;

        if (doc.LoadFile("strings.xml") != tinyxml2::XML_SUCCESS) {
            std::cerr << u8"Ошибка при загрузке XML файла!" << std::endl;
            return "";
        }

        tinyxml2::XMLElement* root = doc.FirstChildElement("resources");
        if (!root) {
            std::cerr << u8"Не найден корневой элемент!" << std::endl;
            return "";
        }

        for (tinyxml2::XMLElement* stringNode = root->FirstChildElement("string"); stringNode != nullptr; stringNode = stringNode->NextSiblingElement("string")) {
            const char* name = stringNode->Attribute("name");
            const char* language = stringNode->Attribute("lang");

            if (name && language && tag == name && lang == language) {
                return stringNode->GetText() ? stringNode->GetText() : "";
            }
        }
        return "";
    };
    static std::string getDataPatch(const std::string& tag, const std::string& lang = "ru") {
        tinyxml2::XMLDocument doc;

        if (doc.LoadFile("Data.xml") != tinyxml2::XML_SUCCESS) {
            std::cerr << u8"Ошибка при загрузке XML файла!" << std::endl;
            return "";
        }

        tinyxml2::XMLElement* root = doc.FirstChildElement("resources");
        if (!root) {
            std::cerr << u8"Не найден корневой элемент!" << std::endl;
            return "";
        }

        for (tinyxml2::XMLElement* stringNode = root->FirstChildElement("string"); stringNode != nullptr; stringNode = stringNode->NextSiblingElement("string")) {
            const char* name = stringNode->Attribute("name");
            const char* language = stringNode->Attribute("lang");

            if (name && language && tag == name && lang == language) {
                return stringNode->GetText() ? stringNode->GetText() : "";
            }
        }
        return "";
    };
    static std::wstring strToWstr(std::string str)
    {
        if (str.empty()) return std::wstring();

        int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
        std::wstring wstrTo(sizeNeeded, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], sizeNeeded);
        return wstrTo;
    };
	static void showError(const std::string& message, const std::string& title) {
		MessageBoxW(NULL, strToWstr(message).c_str(), strToWstr(title).c_str(), MB_OK);
		std::cout << std::endl << title << " : \033[41m" << message << "\n\033[0m";
	};
    static std::string HStringToString(const winrt::hstring& hstr) {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, hstr.c_str(), -1, NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, hstr.c_str(), -1, &strTo[0], size_needed, NULL, NULL);
        return strTo;
    };

};

