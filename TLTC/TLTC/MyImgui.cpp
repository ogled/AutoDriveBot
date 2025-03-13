#include "pch.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "MyImgui.h"
#include "Bluetooth.h"
#include "Helper.h"
#include "iostream"
#include "miniaudio.h"
#include <shellapi.h>
#include <windows.h>
#pragma comment(lib, "Shell32.lib")
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")


using namespace Helper;


MyImgui::MyImgui()
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	if (!glfwInit()) return;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);

	// Получаем ширину и высоту экрана без панели задач
	int width = vidMode->width;
	int height = vidMode->height;

	GLFWwindow* window = glfwCreateWindow(1280,720, getText("WindowStandartName").c_str(), nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	BOOL USE_DARK_MODE = true;
	DwmSetWindowAttribute(glfwGetWin32Window(window), DWMWA_USE_IMMERSIVE_DARK_MODE, &USE_DARK_MODE, sizeof(USE_DARK_MODE));

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowMonitor(window, nullptr, (mode->width - 1280) / 2, (mode->height - 720) / 2, 1280, 720, 0);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 20, NULL, io.Fonts->GetGlyphRangesCyrillic());
	boldBigFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arialbd.ttf", 28, NULL, io.Fonts->GetGlyphRangesCyrillic());
	boldSmallFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arialbd.ttf", 14, NULL, io.Fonts->GetGlyphRangesCyrillic());
	if (!font) {
		std::cerr << u8"Ошибка: не удалось загрузить шрифт!" << std::endl;
	}
	else {
		std::cout << u8"Шрифт успешно загружен!" << std::endl;
	}
	io.Fonts->Build();
	bt = new Bluetooth();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.95f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.30f, 0.80f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.33f, 0.40f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.44f, 0.55f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(47.0f / 255.0f, 43.0f / 255.0f, 63.0f / 255.0f, 1.0f);

	style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_WindowBg];
	style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_TitleBg];
	style.Colors[ImGuiCol_TitleBgCollapsed] = style.Colors[ImGuiCol_TitleBg];
	style.WindowRounding = 10.0f;
	style.FrameRounding = 10.0f;
	style.TabRounding = 10.0f;
	style.PopupRounding = 10.0f;
	style.ChildRounding = 4.0f;
	style.ScrollbarRounding =4.0f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5);
	style.FramePadding = ImVec2(8, 6);
	style.ItemSpacing = ImVec2(8, 6);
	style.ItemInnerSpacing = ImVec2(6, 4);
	style.DisabledAlpha = 0.6f;  // Эффект затухания неактивных элементов

	image = LoadTexture("Wallpaper.png");

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Получаем список отрисовки для фонового слоя
		ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

		// Получаем размеры экрана
		ImVec2 screen_pos = ImGui::GetMainViewport()->Pos;
		ImVec2 screen_size = ImGui::GetMainViewport()->Size;

		// Рисуем изображение во всю область вьюпорта
		draw_list->AddImage((void*)(intptr_t)image,
			screen_pos,
			ImVec2(screen_pos.x + screen_size.x, screen_pos.y + screen_size.y));

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F11)))
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, nullptr, (mode->width - 1280) / 2, (mode->height - 720) / 2, 1280, 720,0);
		}
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu(getText("RobotMenu").c_str())) {
				if (!bt->Connected.value() && (!robotIsConnecting.value() || bt->Ok.value() == false && bt->thr.has_value() && bt->thr.value().joinable()))
				{
					if (ImGui::MenuItem(getText("RobotConnect").c_str()))
					{
						if (bt->thr.has_value() && bt->thr && bt->thr->joinable()) {
							bt->thr->join();
						}
						robotIsConnecting.emplace(true);

						bt->thr.emplace(std::thread(&Bluetooth::StartConnected, bt));
					}
				}
				else if (bt->Connected.value())
				{
					if (ImGui::MenuItem(getText("RobotDisconnect").c_str()))
					{
						bt->DisConnected();
					}

					if (!showInstructionsWindowFlag && ImGui::MenuItem(getText("ShowInstructions").c_str()))
					{
						showInstructionsWindowFlag = true;
					}
					else if (showInstructionsWindowFlag && ImGui::MenuItem(getText("HideInstructions").c_str()))
					{
						showInstructionsWindowFlag = false;
						if (audioPlaying)
						{
							ma_engine_uninit(&engine);
						}
						audioPlaying = false;
					}
				}

				if (!showTasksWindowFlag && ImGui::MenuItem(getText("ShowTask").c_str()))
				{
					showTasksWindowFlag = true;
				}
				else if (showTasksWindowFlag && ImGui::MenuItem(getText("HideTask").c_str()))
				{
					showTasksWindowFlag = false;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu(getText("Program").c_str())) {
				if (ImGui::MenuItem(getText("Debug").c_str()))
				{
					showDebugWindowFlag = !showDebugWindowFlag;
				}
				ImGui::EndMenu();
			}
			if (bt->Connected.value())
			{
				ImGui::NextColumn();
				auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(getText("Connected").c_str()).x
					- ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
				if (posX > ImGui::GetCursorPosX())
					ImGui::SetCursorPosX(posX);
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
				ImGui::Text("%s", getText("Connected").c_str());
				ImGui::PopStyleColor();
			}
			else
			{
				ImGui::NextColumn();
				auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(getText("Disconnected").c_str()).x
					- ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
				if (posX > ImGui::GetCursorPosX())
					ImGui::SetCursorPosX(posX);
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
				ImGui::Text("%s", getText("Disconnected").c_str());
				ImGui::PopStyleColor();
			}
			ImGui::EndMainMenuBar();
		}
		if (bt->con.has_value() && bt->Connected.value() && bt->OkConnect.value() == true)
		{
			robotIsConnecting.emplace(false);
		}
		else if (bt->thr.has_value() && robotIsConnecting.value() && bt->thr.value().joinable() && bt->Ok.value() == true)
		{
			bt->ShowBluetoothWindow();
		}
		showTasksWindow();
		showInstructionsWindow();
		showDebugWindow();
		ImGui::Render();
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void MyImgui::showTasksWindow()
{
	if (showTasksWindowFlag)
	{
		ImGui::SetNextWindowSize(ImVec2(300, 80));
		ImGui::Begin(getText("CurentTask").c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		try
		{
			if (bt->dataFromRobot.value().size() != 0)
			{
				taskWindowsThisText = getText(bt->dataFromRobot.value());
			}
		}
		catch (...) {}

		ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
		ImGui::TextWrapped("%s", taskWindowsThisText.c_str());
		ImGui::PopTextWrapPos();

		ImGui::End();

	}
}

void MyImgui::showInstructionsWindow()
{
	if (showInstructionsWindowFlag && bt->Connected.value() == true)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 40),ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), 0, ImVec2(0.5f, 0.5f));
		ImGui::Begin(getText("Instructions").c_str(), nullptr,  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar| ImGuiWindowFlags_NoScrollWithMouse);

		ImGui::Separator();

		ImGui::BeginChild("LinksChild", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 108));
		ImGui::PushFont(boldSmallFont);
		ImGui::Text(getText("AdditionalInformation").c_str());
		ImGui::PopFont();

		ImGui::Separator();
		for (size_t i = 1; i != links.size() + 1; i++) {
			ImGui::BeginChild(static_cast<int>(i), ImVec2(ImGui::GetContentRegionAvail().x, 60), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			if (ImGui::InvisibleButton("But", ImGui::GetContentRegionAvail()))
			{
				ShellExecute(0, 0, links[i - 1].Link.c_str(), 0, 0, SW_SHOW);
			}
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 52);
			ImGui::PushFont(boldBigFont);
			ImGui::TextWrapped(links[i - 1].Description.c_str());
			ImGui::PopFont();

			std::string link = links[i - 1].Link;
			std::string linkText = link;
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize;
			ImGui::PushFont(boldSmallFont);
			while (1)
			{
				textSize = ImGui::CalcTextSize(linkText.c_str());
				if (textSize.x + windowSize.x / 1.5 >= windowSize.x)
				{
					if (linkText.size() - 5 < 0)
					{
						linkText = "";
					}
					linkText = linkText.erase(linkText.size() - 5) + "...";
				}
				else
				{
					break;
				}
			}
			ImVec2 textPos = ImVec2(windowSize.x - textSize.x - 8,
				ImGui::GetCursorPosY() - 4);
			ImGui::SetCursorPos(textPos);

			ImGui::Text(linkText.c_str());
			ImGui::PopFont();

			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::Separator();
		if (audioPlaying)
		{
			float windowHeight = ImGui::GetWindowHeight();
			float windowWidth = ImGui::GetWindowWidth();

			// Отступы с каждой стороны
			float padding = 20.0f;

			// Устанавливаем вертикальное положение текста
			float textHeight = ImGui::GetTextLineHeightWithSpacing();
			ImGui::SetCursorPosY(windowHeight - textHeight - padding);

			// Получаем доступное пространство для текста
			ImVec2 availSize = ImGui::GetContentRegionAvail();
			float availableWidth = availSize.x;

			// Выравниваем текст по центру
			float textWidth = availableWidth;
			ImGui::SetCursorPosX((windowWidth - textWidth) / 2);
			// Вставляем текст с обертыванием по ширине
			ImGui::TextWrapped(getCurrentSubtitle().c_str());
		}

		ImGui::End();

		try
		{
			if (bt->dataFromRobot.value().size() != 0)
			{
				if (getText(bt->dataFromRobot.value()) == "" || taskWindowsFilesPatch == getDataPatch(bt->dataFromRobot.value()))
				{
					return;
				}
				taskWindowsFilesPatch = getDataPatch(bt->dataFromRobot.value());
			}
			else
			{
				return;
			}
		}
		catch (...) {}

		if (audioPlaying)
		{
			ma_engine_uninit(&engine);
		}
		ma_engine_init(NULL, &engine);
		ma_engine_play_sound(&engine, std::string(taskWindowsFilesPatch + "\\Audio.mp3").c_str(), NULL);
		startTime = std::chrono::steady_clock::now();
		audioPlaying = true;
		subtitles = parseSRT(std::string(taskWindowsFilesPatch + "\\Text.srt"));
		getCurentLinks(std::string(taskWindowsFilesPatch + "\\Links.txt"));
	}
}

void MyImgui::showDebugWindow()
{
	if (showDebugWindowFlag)
	{
		if (!bt->Connected.value())
		{
			bt->Connected.emplace(true);
		}
		ImGui::SetNextWindowSize(ImVec2(280, 80));
		ImGui::Begin(getText("Debug").c_str(), nullptr, ImGuiWindowFlags_NoResize);
		static char buffer[256] = "";  // Буфер для ввода текста
		ImGui::InputText("##input", buffer, sizeof(buffer));

		ImGui::SameLine();  // Разместить кнопку рядом с полем ввода

		if (ImGui::Button("Send")) {
			bt->dataFromRobot.emplace(buffer);
		}
		ImGui::End();
	}
}