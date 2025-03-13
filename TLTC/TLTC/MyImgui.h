#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <iostream>
#include <Windows.h>
#include <tinyxml2.h>
#include "Bluetooth.h"
#include <optional>
#include "miniaudio.h"
#include <fstream>
#include <vector>
#include <regex>
#include <string>
#define MINIAUDIO_IMPLEMENTATION
#include <stb_image.h>

class MyImgui
{
public:
    MyImgui();
private:
    struct Subtitle {
        int start_ms;
        int end_ms;
        std::string text;
    };
    struct Link {
        std::string Link;
        std::string Description;
    };

    Bluetooth* bt;
    std::optional<bool> robotIsConnecting = false;
	bool showTasksWindowFlag = false;
    void showTasksWindow();
    std::string taskWindowsThisText;
    GLuint LoadTexture(const char* filename) {
        int width, height, channels;
        unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
        if (!data) {
            return 0;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        return texture;
    }

    GLuint image;

    //Шрифты
	ImFont* boldBigFont;
    ImFont* boldSmallFont;

	void showInstructionsWindow();
    bool showInstructionsWindowFlag = false;
    std::string taskWindowsFilesPatch;
    std::vector<Subtitle> subtitles;
    ma_engine engine;
    std::chrono::steady_clock::time_point startTime;
    bool audioPlaying = false;
    std::vector<Subtitle> parseSRT(const std::string& filename) {
        std::vector<Subtitle> subtitles2;
        std::ifstream file(filename);
        if (!file) return subtitles2;

        std::string line;
        std::regex timeRegex(R"((\d+):(\d+):(\d+),(\d+))");
        Subtitle sub;
        while (std::getline(file, line)) {
            if (std::regex_search(line, timeRegex)) {
                std::smatch match;
                std::regex_search(line, match, timeRegex);

                if (std::regex_search(line, match, timeRegex)) {
                    int start_h = std::stoi(match[1]);
                    int start_m = std::stoi(match[2]);
                    int start_s = std::stoi(match[3]);
                    int start_ms = std::stoi(match[4]);

                    sub.start_ms = start_h * 3600000 + start_m * 60000 + start_s * 1000 + start_ms;
                }
                line.erase(0, line.find("-"));
                // Для конца времени
                if (std::regex_search(line, match, timeRegex)) {
                    int end_h = std::stoi(match[1]);
                    int end_m = std::stoi(match[2]);
                    int end_s = std::stoi(match[3]);
                    int end_ms = std::stoi(match[4]);

                    sub.end_ms = end_h * 3600000 + end_m * 60000 + end_s * 1000 + end_ms;
                }

                sub.text = "";
                while (std::getline(file, line) && !line.empty()) {
                    sub.text += line + "\n";
                }

                subtitles2.push_back(sub);
            }
        }

        return subtitles2;
    }
    std::string getCurrentSubtitle() {
        if (!audioPlaying) return "";
        int elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count();

        for (const auto& sub : subtitles) {
            if (elapsedMs >= sub.start_ms && elapsedMs <= sub.end_ms) {
                return sub.text;
            }
        }
        return "";
    }
    std::vector<Link> links;
    void getCurentLinks(const std::string& filename)
    {
        std::ifstream file(filename);
        std::string line;
		links.clear();
        while (std::getline(file, line)) {
            if (line.find("(\"") - 1 <= 0 && line.find("\")") <= 0)
                continue;

            Link temp;
            temp.Description = line.substr(0, line.find("(\"") - 1);
            temp.Link = line.substr(line.find("(\"") + 2, line.find("\")") - (line.find("(\"") + 2));
            links.push_back(temp);
        }
    }


    bool showDebugWindowFlag = false;
	void showDebugWindow();
};

