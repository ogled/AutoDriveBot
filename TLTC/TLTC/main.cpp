#include "pch.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <Windows.h>
#include "MyImgui.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    MyImgui gui = MyImgui();
    return 0;
}
