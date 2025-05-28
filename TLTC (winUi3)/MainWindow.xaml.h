#pragma once

#include "MainWindow.g.h"

namespace winrt::TLTC__winUi3_::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        HWND GetWindowHandle();
        MainWindow();
        int32_t MyProperty();
        void MyProperty(int32_t value);
        void mainView_BackRequested(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewBackRequestedEventArgs const& args);
        void mainView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void mainView_SelectionChanged(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args);
    };
}

namespace winrt::TLTC__winUi3_::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
