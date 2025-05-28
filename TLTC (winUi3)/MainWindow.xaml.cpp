#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#include "winrt/Microsoft.UI.Xaml.Media.h"
#include "winrt/Microsoft.UI.Xaml.h"
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/microsoft.ui.xaml.interop.h>
#endif
#include <microsoft.ui.xaml.window.h>
#include <winrt/Windows.Graphics.h>
#include <Windows.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.ViewManagement.h>
#pragma comment(lib, "Comctl32.lib")
#include <CommCtrl.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include "LinkWithRobot.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Input;
using namespace Microsoft::UI::Xaml::Media;
using namespace Microsoft::UI::Windowing;
using namespace Microsoft::UI::Xaml::Interop;
using namespace Windows::Graphics;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::TLTC__winUi3_::implementation
{
    HWND MainWindow::GetWindowHandle()
    {
        auto nativeWindow = this->try_as<IWindowNative>();
        HWND hwnd{ nullptr };
        if (nativeWindow)
        {
            nativeWindow->get_WindowHandle(&hwnd);
        }
        return hwnd;
    }
    int32_t MainWindow::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainWindow::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
winrt::TLTC__winUi3_::implementation::MainWindow::MainWindow()
{
    InitializeComponent();
    HWND hwnd = GetWindowHandle();
    Window window = *this;
    window.ExtendsContentIntoTitleBar(true);
    // Получаем AppWindow для управления окном
    auto windowId = winrt::Microsoft::UI::GetWindowIdFromWindow(hwnd);
    auto appWindow = AppWindow::GetFromWindowId(windowId);

    SetWindowSubclass(hwnd, [](HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR dwRefData) -> LRESULT
        {
            if (uMsg == WM_GETMINMAXINFO)
            {
                MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
                auto* minSize = reinterpret_cast<std::pair<int, int>*>(dwRefData);
                if (minSize)
                {
                    mmi->ptMinTrackSize.x = minSize->first;
                    mmi->ptMinTrackSize.y = minSize->second;
                }
                return 0;
            }
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }, 1, reinterpret_cast<DWORD_PTR>(new std::pair<int, int>(480, 540)));
}
void winrt::TLTC__winUi3_::implementation::MainWindow::mainView_BackRequested(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewBackRequestedEventArgs const& args)
{
    if (ContentFrame().CanGoBack())
    {
        ContentFrame().GoBack();
    }
}

void winrt::TLTC__winUi3_::implementation::MainWindow::mainView_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    auto items = mainView().MenuItems();
    for (auto const& item : items)
    {
        if (auto navigationItem = item.try_as<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>())
        {
            if (navigationItem.Content().try_as<winrt::hstring>() == L"Главная")
            {
                mainView().SelectedItem(navigationItem);
                break;
            }
        }
    }
}

void winrt::TLTC__winUi3_::implementation::MainWindow::mainView_SelectionChanged(winrt::Microsoft::UI::Xaml::Controls::NavigationView const& sender, winrt::Microsoft::UI::Xaml::Controls::NavigationViewSelectionChangedEventArgs const& args)
{
    auto selectedItem = args.SelectedItem();
    if (auto navigationItem = selectedItem.try_as<winrt::Microsoft::UI::Xaml::Controls::NavigationViewItem>())
    {
        auto content = navigationItem.Content().try_as<winrt::hstring>();
        if (content == L"Главная")
        {
            ContentFrame().Navigate(xaml_typename<TLTC__winUi3_::Home>());
        }
        else if (content == L"Информация")
        {
            LinkWithRobot* link = new LinkWithRobot();
            uint64_t ptr = reinterpret_cast<uint64_t>(link);
            ContentFrame().Navigate(
                winrt::xaml_typename<TLTC__winUi3_::MainPageApp>(),
                winrt::box_value(ptr),
                winrt::Microsoft::UI::Xaml::Media::Animation::DrillInNavigationTransitionInfo::DrillInNavigationTransitionInfo()
            );
        }

    }
}
