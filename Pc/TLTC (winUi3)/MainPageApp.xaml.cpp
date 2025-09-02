#include "pch.h"
#include "MainPageApp.xaml.h"
#if __has_include("MainPageApp.g.cpp")
#include "MainPageApp.g.cpp"
#endif
#include <winrt/Microsoft.UI.Text.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Windows.Graphics.h>
#include "Assets/Animated/AnimatedVisuals.RobotGoAnim.h"
#include "Assets/Animated/AnimatedVisuals.RobotSleepAnim.h"
#include <fstream>
#include <sstream>
#include "nlohmann/json.hpp"
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Media.SpeechSynthesis.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/impl/Windows.UI.Xaml.Controls.0.h>
#include <shellapi.h>;
#include <cstdlib>
#include <winrt/Microsoft.Web.WebView2.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <microsoft.ui.xaml.window.h>
#include <windowsx.h>
#include "KeyHandler.h"
#include <winrt/Windows.Gaming.Input.h>

#pragma comment(lib, "Shell32.lib")

using namespace winrt;
using namespace Microsoft::UI::Xaml;
namespace fs = std::filesystem;
using json = nlohmann::json;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.


namespace winrt::TLTC__winUi3_::implementation
{
    int32_t MainPageApp::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPageApp::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::ChangeColor_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    ColorPickerDialog().PrimaryButtonClick({this, &MainPageApp::ColorPickerDialog_PrimaryButtonClick});
    ColorPickerDialog().SecondaryButtonClick({this, &MainPageApp::ColorPickerDialog_SecondaryButtonClick});
    ColorPickerDialog().ShowAsync();
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::ProMode_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    if(ProMode().IsChecked())
    {
        ProModeGrid().Visibility(Visibility::Visible);
        ProModeGrid().IsHitTestVisible(true);

        StandartModeGrid().Visibility(Visibility::Collapsed);
        StandartModeGrid().IsHitTestVisible(false);
    }
    else
    {
        StandartModeGrid().Visibility(Visibility::Visible);
        StandartModeGrid().IsHitTestVisible(true);

        ProModeGrid().Visibility(Visibility::Collapsed);
        ProModeGrid().IsHitTestVisible(false);
    }
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::ColorPickerDialog_SecondaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args)
{
    sender.Hide();
}
void winrt::TLTC__winUi3_::implementation::MainPageApp::ColorPickerDialog_PrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args)
{
    winrt::Windows::UI::Color selectedColor = ColorPicker().Color();

    std::string r = std::to_string(selectedColor.R);
    std::string g = std::to_string(selectedColor.G);
    std::string b = std::to_string(selectedColor.B);

    link->sendMesenge("rgb(" + r + "," + g + "," + b + ");");

    auto settings = winrt::Windows::Storage::ApplicationData::Current().LocalSettings();

    if (settings.Values().HasKey(L"SavedColor"))
    {
        settings.Values().Remove(L"SavedColor");
    }

    auto composite = winrt::Windows::Storage::ApplicationDataCompositeValue();
    composite.Insert(L"R", winrt::box_value<uint32_t>(selectedColor.R));
    composite.Insert(L"G", winrt::box_value<uint32_t>(selectedColor.G));
    composite.Insert(L"B", winrt::box_value<uint32_t>(selectedColor.B));
    composite.Insert(L"A", winrt::box_value<uint32_t>(selectedColor.A));

    settings.Values().Insert(L"SavedColor", composite);
    sender.Hide();
}

winrt::Windows::UI::Color winrt::TLTC__winUi3_::implementation::MainPageApp::LoadColor()
{
    auto settings = winrt::Windows::Storage::ApplicationData::Current().LocalSettings();

    if (!settings.Values().HasKey(L"SavedColor"))
    {
        return winrt::Microsoft::UI::Colors::LightGoldenrodYellow();
    }

    try
    {
        auto value = settings.Values().Lookup(L"SavedColor");

        if (value == nullptr)
        {
            settings.Values().Remove(L"SavedColor");
            return winrt::Microsoft::UI::Colors::LightGoldenrodYellow();
        }

        auto composite = value.try_as<winrt::Windows::Storage::ApplicationDataCompositeValue>();
        if (!composite)
        {
            settings.Values().Remove(L"SavedColor");
            return winrt::Microsoft::UI::Colors::LightGoldenrodYellow();
        }

        const std::array keys = { L"R", L"G", L"B", L"A" };
        for (const auto& key : keys)
        {
            if (!composite.HasKey(key))
            {
                settings.Values().Remove(L"SavedColor");
                return winrt::Microsoft::UI::Colors::LightGoldenrodYellow();
            }
        }

        try
        {
            uint8_t r = static_cast<uint8_t>(winrt::unbox_value<uint32_t>(composite.Lookup(L"R")));
            uint8_t g = static_cast<uint8_t>(winrt::unbox_value<uint32_t>(composite.Lookup(L"G")));
            uint8_t b = static_cast<uint8_t>(winrt::unbox_value<uint32_t>(composite.Lookup(L"B")));
            uint8_t a = static_cast<uint8_t>(winrt::unbox_value<uint32_t>(composite.Lookup(L"A")));

            return { a, r, g, b };
        }
        catch (...)
        {
            settings.Values().Remove(L"SavedColor");
            return winrt::Microsoft::UI::Colors::LightGoldenrodYellow();
        }
    }
    catch (...)
    {
        settings.Values().Remove(L"SavedColor");
        return winrt::Microsoft::UI::Colors::LightGoldenrodYellow();
    }
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::RobotCheckState(std::vector<std::wstring> args)
{
    StopButton().IsEnabled(true);
    link->sendMesenge("connectTrue();");
    Sleep(300);
    StopButton().IsEnabled(true);
    PauseResumeButton().IsEnabled(true);
    ForNextContinueButton().IsEnabled(false);

    winrt::Windows::UI::Color selectedColor = ColorPicker().Color();

    std::string r = std::to_string(selectedColor.R);
    std::string g = std::to_string(selectedColor.G);
    std::string b = std::to_string(selectedColor.B);

    link->sendMesenge("rgb(" + r + "," + g + "," + b + ");");


    if (isJoysticMode)
    {
        Sleep(300);
        auto fakeArgs = winrt::Microsoft::UI::Xaml::RoutedEventArgs();
        RobotJoystick_Click(RobotJoystick(), fakeArgs);
    }
}
//State(speed,angle,battery);
void winrt::TLTC__winUi3_::implementation::MainPageApp::FromRobotState()
{
    SpeedText().Text(winrt::to_hstring(sensorsValues[0]) + L" мм/с");
    AngleText().Text(winrt::to_hstring(sensorsValues[1]) + L" °");
    BatteryText().Text(winrt::to_hstring(sensorsValues[2]) + L" %");
    WheelAngleText().Text(winrt::to_hstring(sensorsValues[4]) + L" °");
}
winrt::Windows::Foundation::IAsyncAction winrt::TLTC__winUi3_::implementation::MainPageApp::PlayText(winrt::hstring text)
{
    m_player = winrt::Windows::Media::Playback::MediaPlayer();
    m_player.AudioCategory(winrt::Windows::Media::Playback::MediaPlayerAudioCategory::Speech);
    winrt::Windows::Media::SpeechSynthesis::SpeechSynthesizer synthesizer;
    auto voices = winrt::Windows::Media::SpeechSynthesis::SpeechSynthesizer::SpeechSynthesizer::AllVoices();

    hstring selectedName = unbox_value<hstring>(VoiceComboBox().SelectedItem().try_as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>().Content());
    for (auto const& voice : voices)
    {
        if (voice.DisplayName() == selectedName)
        {
            synthesizer.Voice(voice);
            break;
        }
    }

    bool is_ssml = (winrt::to_string(text).find("<speak", 0) == 0);
    winrt::Windows::Media::SpeechSynthesis::SpeechSynthesisStream stream{ nullptr };

    if (is_ssml) {
        std::string str2 = winrt::to_string(text);
        try {
            stream = co_await synthesizer.SynthesizeSsmlToStreamAsync(winrt::to_hstring(str2));
        }
        catch (winrt::hresult_error const& ex) {
            OutputDebugStringW((L"Ошибка синтеза: " + ex.message()).c_str());
            co_return;
        }
    }
    else {
        stream = co_await synthesizer.SynthesizeTextToStreamAsync(text);
    }

    m_player.MediaEnded([this](auto&&, auto&&) {
        link->sendMesenge("EndMedia();");
        });

    winrt::Windows::Storage::Streams::IRandomAccessStream audioStream = stream;

    m_player.SetStreamSource(stream);
    m_player.Play();
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::joysticStart()
{
    if (!joystick.isInitialize)
    {
        if (joystick.Initialize()) {
            OutputDebugStringW(L"Joystick connected");

            Microsoft::UI::Xaml::DispatcherTimer timer;
            timer.Interval(std::chrono::milliseconds(delaySendControllerState));
            timer.Tick([this](auto&&, auto&&) {
                for (auto& msg : joystick.Update()) {
                    link->sendMesenge(msg);
                    OutputDebugStringA(("Joystic change: " + msg).c_str());
                }
                });
            timer.Start();
        }
        else {
            OutputDebugStringW(L"No joystick found");
        }
    }
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::parse_config_file(const std::string& filename)
{
    auto folder = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation();
    std::string fullPath = winrt::to_string(folder.Path()).c_str();
    fullPath += "\\" + filename;

    std::ifstream input(fullPath);
    if (input)
    {
        try {
            json data = json::parse(input);

            for (const auto& item : data) {
                Config config;
                config.title = item["Title"].get<std::string>();
                config.command = item["Command"].get<std::string>();
                config.image = item["Image"].get<std::string>();
                config.textSound = item["TextSound"].get<std::string>();
                for (const auto& step_item : item["Steps"]) {
                    Step step;
                    step.name = step_item["Name"].get<std::string>();
                    step.description = step_item["Description"].get<std::string>();
                    config.steps.push_back(step);
                }

                commandsOther.push_back(config);
            }
        }
        catch(...){}
    }
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::SleepRobotState(std::vector<std::wstring> args)
{
    StopButton().IsEnabled(false);
    PauseResumeButton().IsEnabled(false);
    ForNextContinueButton().IsEnabled(false);
    YoutubeView().Source(winrt::Windows::Foundation::Uri{ L"about:blank"});
    PauseResumeIcon().Glyph(L"\uF8AE");
    isPaused = false;
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::OnPrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args)
{
    auto selected = VoiceComboBox().SelectedItem().try_as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>();
    if (VoiceComboBox().SelectedItem() != nullptr && selected != nullptr)
    {
        auto settings = winrt::Windows::Storage::ApplicationData::Current().LocalSettings();

        winrt::Windows::Storage::ApplicationDataCompositeValue composite;
        composite.Insert(L"VoiceText", winrt::box_value(selected.Content()));
        composite.Insert(L"IsToggleSound", winrt::box_value(isToggleSound));

        settings.Values().Insert(L"VoiceSettings", composite);
    }
}
void winrt::TLTC__winUi3_::implementation::MainPageApp::OnTestVoiceClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    PlayText(TestTextBox().Text());
}

winrt::TLTC__winUi3_::implementation::MainPageApp::MainPageApp()
{
     InitializeComponent();
     parse_config_file("Config.json");
	 ColorPicker().Color(LoadColor());

     auto voices = winrt::Windows::Media::SpeechSynthesis::SpeechSynthesizer::AllVoices();
     VoiceComboBox().Items().Clear();
     for (const auto& voice : voices)
     { 
         if (voice.Language() == L"ru-RU")
         {
             winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem item;
             item.Content(box_value(voice.DisplayName()));
             VoiceComboBox().Items().Append(item);
         }
     }
     auto settings = winrt::Windows::Storage::ApplicationData::Current().LocalSettings();
     if (settings.Values().HasKey(L"VoiceSettings"))
     {
         auto value = settings.Values().Lookup(L"VoiceSettings");
         auto composite = value.try_as<winrt::Windows::Storage::ApplicationDataCompositeValue>();

         if (!composite || !composite.HasKey(L"VoiceText") || !composite.HasKey(L"IsToggleSound"))
         {
             settings.Values().Remove(L"VoiceSettings");
         }
         else
         {
             hstring savedVoice = unbox_value<hstring>(composite.Lookup(L"VoiceText"));
             if (!savedVoice.empty())
             {
                 for (uint32_t i = 0; i < VoiceComboBox().Items().Size(); ++i)
                 {

                     if (unbox_value<hstring>(VoiceComboBox().Items().GetAt(i).try_as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>().Content()) == savedVoice)
                     {
                         VoiceComboBox().SelectedIndex(i);
                         break;
                     }
                 }
                 isToggleSound = winrt::unbox_value<bool>(composite.Lookup(L"IsToggleSound"));
             }
             else
             {
                 settings.Values().Remove(L"VoiceSettings");
             }
             
         }
     }
     if (VoiceComboBox().SelectedIndex() < 0)
     {
         VoiceComboBox().SelectedIndex(0);
     }
     ToggleSound().IsChecked(isToggleSound);
     Mp4Player().MediaPlayer().IsMuted(true);

     commands = {
        {L"CheckState", [this](std::vector<std::wstring> args) {
            this->RobotCheckState(args);
        }},
        {L"msg", [this](std::vector<std::wstring> args) {
            MessageBoxW(nullptr, args[0].c_str(), L"Сообщение от робота", MB_OK);
        }},
        {L"SleepRobot", [this](std::vector<std::wstring> args) {
            this->SleepRobotState(args);
        }}
     };

     if (!m_timer)
     {
         m_timer = winrt::Microsoft::UI::Xaml::DispatcherTimer();
         m_timer.Interval(std::chrono::milliseconds(5));
         m_timer.Tick({ this, &MainPageApp::checkValueFromRobot });
     }
     m_timer.Start();

     s_instance = this;

     joysticStart();
}

winrt::TLTC__winUi3_::implementation::MainPageApp::~MainPageApp()
{
    if (m_timer)
    {
        m_timer.Stop();
        m_timer = nullptr;
    }
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::checkValueFromRobot(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&)
{
    if (!link->errorMesenge.empty())
    {
        ConsoleOutput().Text(ConsoleOutput().Text() + L"> " + link->errorMesenge);
        link->errorMesenge = L"";
        auto scrollViewer = ConsoleOutput().FindName(L"ScrollViewer").as<winrt::Microsoft::UI::Xaml::Controls::ScrollViewer>();
        if (scrollViewer)
        {
            scrollViewer.ChangeView(nullptr, scrollViewer.ScrollableHeight(), nullptr);
        }
    }
    if (!link->fromRobot.empty())
    {
        if (link->fromRobot.front().find(L"State") == -1)
        {
            ConsoleOutput().Text(ConsoleOutput().Text() + L"> " + link->fromRobot.front());
        }
        auto scrollViewer = ConsoleOutput().FindName(L"ScrollViewer").as<winrt::Microsoft::UI::Xaml::Controls::ScrollViewer>();
        if (scrollViewer)
        {
            scrollViewer.ChangeView(nullptr, scrollViewer.ScrollableHeight(), nullptr);
        }

        if (link->fromRobot.front().find(L"RECEIVED: ") != -1)
        {
            for (int i = 0;; i++)
            {
                if (i == 0)
                {
                    std::wstring command = link->fromRobot.front().substr(link->fromRobot.front().find(L"RECEIVED: ") + 10);

                    if (command.rfind(L"S:", 0) == 0)
                    {
                        size_t commaPos = command.find(L',');
                        if (commaPos != std::wstring::npos)
                        {
                            std::wstring sensorPart = command.substr(2, commaPos - 2);
                            int sensorIndex = -1;
                            if (!sensorPart.empty() && sensorPart[0] == L's')
                            {
                                sensorIndex = std::stoi(sensorPart.substr(1));
                            }

                            std::wstring valuePart = command.substr(commaPos + 1);
                            int sensorValue = std::stoi(valuePart);

                            sensorsValues[sensorIndex] = sensorValue;

                            ConsoleOutput().Text(ConsoleOutput().Text() +
                                L"> Sensor " + std::to_wstring(sensorIndex) +
                                L" = " + std::to_wstring(sensorValue) + L"\n");

                            auto scrollViewer = ConsoleOutput().FindName(L"ScrollViewer").as<winrt::Microsoft::UI::Xaml::Controls::ScrollViewer>();
                            if (scrollViewer)
                            {
                                scrollViewer.ChangeView(nullptr, scrollViewer.ScrollableHeight(), nullptr);
                            }
                            OutputDebugStringW((L"Sensor change: " + command).c_str());

                            FromRobotState();
                            break;
                        }
                    }
                }
                else if (i == 1)
                {
                    std::wstring command = link->fromRobot.front().substr(link->fromRobot.front().find(L"RECEIVED: ") + 10);
                    size_t parenOpen = command.find(L'(');
                    size_t parenClose = command.find(L')');
                    size_t semicolon = command.find(L';');

                    if (parenOpen == std::wstring::npos ||
                        parenClose == std::wstring::npos ||
                        semicolon == std::wstring::npos ||
                        parenClose > semicolon) {
                        continue;
                    }

                    std::wstring command2 = command.substr(0, parenOpen);

                    auto it = commands.find(command2);
                    if (it == commands.end()) {
                        continue;
                    }

                    std::vector<std::wstring> args;
                    size_t argStart = parenOpen + 1;
                    size_t argEnd = parenClose;
                    size_t current = argStart;
                    size_t tokenStart = current;

                    while (current <= argEnd) {
                        if (command[current] == L',' || current == argEnd) {
                            args.emplace_back(command.substr(tokenStart, current - tokenStart));
                            tokenStart = current + 1;
                        }
                        ++current;
                    }

                    it->second(args);
                    break;
                }
                else if (i == 2)
                {
                    std::wstring command = link->fromRobot.front().substr(link->fromRobot.front().find(L"RECEIVED: ") + 10);
                    size_t parenOpen = command.find(L'(');
                    size_t parenClose = command.find(L')');
                    size_t semicolon = command.find(L';');

                    if (parenOpen == std::wstring::npos ||
                        parenClose == std::wstring::npos ||
                        semicolon == std::wstring::npos ||
                        parenClose > semicolon) {
                        continue;
                    }

                    std::wstring command2 = command.substr(0, parenOpen);
                    Config it;
                    for (Config con : commandsOther)
                    {
                        if (winrt::to_hstring(con.command) == winrt::hstring(command2))
                        {
                            it = con;
                        }
                    }
                    if (it.command.empty())
                    {
                        continue;
                    }
                    Windows::Foundation::Collections::IVector<IInspectable> items = single_threaded_vector<IInspectable>();
                    MainText().Text(winrt::to_hstring(it.title));
                    for (Step st : it.steps)
                    {
                        items.Append(box_value(winrt::to_hstring(st.name.c_str())));
                    }
                    StepsList().ItemsSource(items);
                    activeCommand = it;
                    StepsList().SelectedIndex(0);
                    StepsList_SelectionChanged(nullptr, nullptr);
                    std::string image = it.image;

                    DispatcherQueue().TryEnqueue([=]() {
                        try { LottiePlayer().Source(nullptr); }
                        catch (...) {}
                        try { if (auto mp = Mp4Player().MediaPlayer()) mp.Source(nullptr); }
                        catch (...) {}
                        try { StaticPngImage().Source(nullptr); }
                        catch (...) {}
                        try { YoutubeView().Source(nullptr); }
                        catch (...) {}
                        try { LottiePlayer().Visibility(Visibility::Collapsed); }
                        catch (...) {}
                        try { Mp4Player().Visibility(Visibility::Collapsed); }
                        catch (...) {}
                        try { StaticPngImage().Visibility(Visibility::Collapsed); }
                        catch (...) {}
                        try { YoutubeView().Visibility(Visibility::Collapsed); }
                        catch (...) {}
                        });

                    if (image.find("AnimatedVisuals::") != std::string::npos)
                    {
                        image.erase(0, image.find("AnimatedVisuals::") + sizeof("AnimatedVisuals::") - 1);

                        DispatcherQueue().TryEnqueue([image, this]() {
                            try {
                                LottiePlayer().Visibility(Visibility::Visible);
                                if (image == "RobotGoAnim") {
                                    LottiePlayer().Source(AnimatedVisuals::RobotGoAnim());
                                }
                                
                            }
                            catch (...) {}
                            });
                    }
                    else if (!image.empty())
                    {
                        std::string ext = "";
                        size_t dotPos = image.find_last_of('.');
                        if (dotPos != std::string::npos) {
                            ext = image.substr(dotPos);
                            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        }

                        const std::vector<std::string> videoAndAudioExts = {
                            ".mp4", ".mov", ".webm", ".avi", ".wmv", ".mkv", ".m4v", ".mpg", ".mpeg",
                            ".mp3", ".wav", ".flac", ".aac", ".ogg", ".wma", ".m4a"
                        };
                        const std::vector<std::string> imageExts = {
                            ".png", ".jpg", ".jpeg", ".bmp", ".gif", ".webp", ".tif", ".tiff"
                        };
                        const std::vector<std::string> urlExts = { "https", "http", "file" };

                        bool isVideo = std::find(videoAndAudioExts.begin(), videoAndAudioExts.end(), ext) != videoAndAudioExts.end();
                        bool isImage = std::find(imageExts.begin(), imageExts.end(), ext) != imageExts.end();
                        bool isUrl = false;

                        if (!isVideo && !isImage)
                        {
                            dotPos = image.find_first_of(':');
                            if (dotPos != std::string::npos) {
                                ext = image.substr(0, dotPos);
                                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                            }
                            isUrl = std::find(urlExts.begin(), urlExts.end(), ext) != urlExts.end();
                        }

                        if (isVideo)
                        {
                            DispatcherQueue().TryEnqueue([image, this]() {
                                try {
                                    LottiePlayer().Visibility(Visibility::Collapsed);
                                    StaticPngImage().Visibility(Visibility::Collapsed);
                                    YoutubeView().Visibility(Visibility::Collapsed);
                                    Mp4Player().Visibility(Visibility::Visible);
                                    Mp4Player().UpdateLayout();

                                    auto mediaPlayer = winrt::Windows::Media::Playback::MediaPlayer();
                                    mediaPlayer.IsLoopingEnabled(true);
                                    mediaPlayer.IsMuted(true);
                                    mediaPlayer.AutoPlay(false);

                                    Mp4Player().SetMediaPlayer(mediaPlayer);

                                    winrt::hstring hpath = L"ms-appx:///" + winrt::to_hstring(image);
                                    auto source = winrt::Windows::Media::Core::MediaSource::CreateFromUri(winrt::Windows::Foundation::Uri(hpath));
                                    mediaPlayer.Source(source);

                                    mediaPlayer.Play();
                                }
                                catch (...) {}
                                });
                        }
                        else if (isUrl)
                        {
                            DispatcherQueue().TryEnqueue([image, ext, this]() {
                                try {
                                    LottiePlayer().Visibility(Visibility::Collapsed);
                                    StaticPngImage().Visibility(Visibility::Collapsed);
                                    Mp4Player().Visibility(Visibility::Collapsed);
                                    YoutubeView().Visibility(Visibility::Visible);

                                    std::string localImage = image;
                                    if (ext == "file")
                                    {
                                        const std::string msAppxPrefix = "{ms-appx:///}";
                                        size_t pos = 0;
                                        while ((pos = localImage.find(msAppxPrefix, pos)) != std::string::npos) {
                                            std::string relativePath = localImage.substr(pos + msAppxPrefix.length());
                                            auto folder = winrt::Windows::ApplicationModel::Package::Current().InstalledLocation();
                                            std::string fullPath = winrt::to_string(folder.Path());
                                            fullPath += "\\" + relativePath;
                                            localImage.replace(pos, msAppxPrefix.length() + relativePath.length(), fullPath);
                                            pos += fullPath.length();
                                        }
                                    }
                                    YoutubeView().Source(winrt::Windows::Foundation::Uri{ winrt::to_hstring(localImage) });
                                }
                                catch (...) {}
                                });
                        }
                        else if (isImage)
                        {
                            DispatcherQueue().TryEnqueue([image, this]() {
                                try {
                                    LottiePlayer().Visibility(Visibility::Collapsed);
                                    Mp4Player().Visibility(Visibility::Collapsed);
                                    YoutubeView().Visibility(Visibility::Collapsed);
                                    StaticPngImage().Visibility(Visibility::Visible);

                                    winrt::Windows::Foundation::Uri pngUri{ winrt::hstring(L"ms-appx:///" + winrt::to_hstring(image)) };
                                    winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage bitmap;
                                    bitmap.UriSource(pngUri);
                                    StaticPngImage().Source(bitmap);
                                }
                                catch (...) {}
                                });
                        }
                    }

                    if (isToggleSound)
                    {
                        PlayText(winrt::to_hstring(it.textSound));
                    }
                    else
                    {
                        link->sendMesenge("EndMedia();");
                    }
                    break;
                }
                else
                {
                    break;
                }
            }
        }
        link->fromRobot.pop();
    }
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::SendButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{

    winrt::hstring text;
    SendTextBox().Document().GetText(
        winrt::Microsoft::UI::Text::TextGetOptions::None, text
    );
    if (text.c_str() == L"cls")
    {
        ConsoleOutput().Text(L"");
    }
    else
    {
        link->sendMesenge(winrt::to_string(text.c_str()));
        SendTextBox().TextDocument().SetText(winrt::Microsoft::UI::Text::TextSetOptions::None, L"");
    }
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::MenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    AboutDialog().ShowAsync();
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::StepsList_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e)
{
    int index = StepsList().SelectedIndex();
    if (activeCommand.command.empty() || activeCommand.steps.size() - 1 < index)
    {
        return;
    }

    OtherInfirmation().Text(winrt::to_hstring(activeCommand.steps[index].description));
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::ToggleMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    SoundSettings().ShowAsync();
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::ToggleSound_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    isToggleSound = ToggleSound().IsChecked();
    OnPrimaryButtonClick(nullptr, nullptr);
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::DeveloperOpenFolder_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    char exePath[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string folderPath(exePath);
    size_t pos = folderPath.find_last_of("\\/");
    if (pos == std::string::npos)
    {
        return;
    }
    folderPath = folderPath.substr(0, pos);
    ShellExecuteA(NULL, "open", folderPath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::IsAreTransportControlsEnabled_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    DispatcherQueue().TryEnqueue([this]()
        {
            Mp4Player().AreTransportControlsEnabled(IsAreTransportControlsEnabled().IsChecked());
        });
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::DeveloperRobotEmulated_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    DeveloperRobotEmulatedDialog().ShowAsync();
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    link->fromRobot.push(winrt::hstring(L"RECEIVED: " + ComandToRobotDeveloperComboBox().Text() + L"\r\n").c_str());
    ComandToRobotDeveloperComboBox().Text(L"");
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::PauseResumeButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    isPaused = !isPaused;
    PauseResumeIcon().Glyph(isPaused ? L"\uF5B0" : L"\uF8AE");
    if (isPaused)
    {
        ForNextContinueButton().IsEnabled(true);
        link->sendMesenge("Pause();");
    }
    else
    {
        ForNextContinueButton().IsEnabled(false);
        link->sendMesenge("Resume();");
    }
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::StopButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    link->sendMesenge("BrakeProgram();");
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::ForNextContinueButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    link->sendMesenge("NextStep();");
}

void winrt::TLTC__winUi3_::implementation::MainPageApp::RobotJoystick_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    joysticStart();
    isJoysticMode = RobotJoystick().IsChecked();

    if (isJoysticMode)
    {
        link->sendMesenge("StartJM();");
    }
    else
    {
        link->sendMesenge("StopJM();");
    }
}