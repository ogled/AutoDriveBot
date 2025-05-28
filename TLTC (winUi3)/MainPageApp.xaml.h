#pragma once

#include "MainPageApp.g.h"
#include "LinkWithRobot.h"
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <functional>

namespace winrt::TLTC__winUi3_::implementation
{
    struct MainPageApp : MainPageAppT<MainPageApp>
    {
    private:
            struct Step {
                std::string name;
                std::string description;
            };
            struct Config {
                std::string title;
                std::string command;
                std::string image;
                std::string textSound;
                std::vector<Step> steps;
            };

        winrt::Windows::Media::Playback::MediaPlayer m_player{ nullptr };
        bool isToggleSound = false;
        bool isPaused = false;
        winrt::Microsoft::UI::Xaml::DispatcherTimer m_timer{ nullptr };
        std::vector<Config> commandsOther;
        Config activeCommand;
        LinkWithRobot* link;

        winrt::Windows::UI::Color LoadColor();
        void parse_config_file(const std::string& filename);
        void SleepRobotState(std::vector<std::wstring> args);
        void RobotCheckState(std::vector<std::wstring> args);
        void FromRobotState(std::vector<std::wstring> args);

        std::map<std::wstring, std::function<void(std::vector<std::wstring>)>> commands;
        winrt::Windows::Foundation::IAsyncAction PlayText(winrt::hstring text);

    public:
        MainPageApp();
        ~MainPageApp();

        int32_t MyProperty();
        void MyProperty(int32_t value);
        void checkValueFromRobot(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&);
        void ChangeColor_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ProMode_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ColorPickerDialog_SecondaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);
        void ColorPickerDialog_PrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);
        void SendButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void MenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void StepsList_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void ToggleMenuFlyoutItem_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OnPrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender, winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);
        void OnTestVoiceClicked(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void OnNavigatedTo(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e)
        {
            if (auto boxedPtr = e.Parameter().try_as<Windows::Foundation::IPropertyValue>())
            {
                uint64_t raw = boxedPtr.GetUInt64();
                link = reinterpret_cast<LinkWithRobot*>(raw);
            }
        }
        void ToggleSound_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DeveloperOpenFolder_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void IsAreTransportControlsEnabled_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DeveloperRobotEmulated_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void PauseResumeButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void StopButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ForNextContinueButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::TLTC__winUi3_::factory_implementation
{
    struct MainPageApp : MainPageAppT<MainPageApp, implementation::MainPageApp>
    {
    };
}
