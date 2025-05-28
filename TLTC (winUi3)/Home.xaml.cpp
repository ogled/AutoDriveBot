#include "pch.h"  
#include "Home.xaml.h"
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#if __has_include("Home.g.cpp")  
#include "Home.g.cpp"  
#endif  
#include <mutex>
#include <optional>
#include "LinkWithRobot.h"
#include "sstream"
#include <winrt/impl/Windows.UI.Xaml.Controls.0.h>
#include <filesystem>
#include "Assets/Animated/AnimatedVisuals.CheckmarkAnim.h"
#include "winrt/Microsoft.Windows.System.h"
#include <Windows.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Windows.UI.Xaml.Interop.h>


#pragma comment(lib, "winmm.lib")

using namespace winrt;  
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Interop;
using namespace Windows::Foundation::Collections; 
using namespace Windows::Devices::Bluetooth::Advertisement;
using namespace winrt::Microsoft::UI::Xaml::Media::Animation;

template <typename ancestor_type>
auto find_ancestor(::winrt::Microsoft::UI::Xaml::DependencyObject root) noexcept
{
    auto ancestor{ root.try_as<ancestor_type>() };
    while (!ancestor && root)
    {
        root = ::winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper::GetParent(root);
        ancestor = root.try_as<ancestor_type>();
    }
    return ancestor;
}


namespace winrt::TLTC__winUi3_::implementation  
{
    Home::Home()
    {
        InitializeComponent();
        if (m_watcher)
        {
            m_watcher.Stop();
            m_watcher = nullptr;
        }

		updateElementsBluetoothDevicesComboBox();
        m_watcher = winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher();
        m_watcher.ScanningMode(BluetoothLEScanningMode::Active);
        m_watcher.Received([this](BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args) {
            this->StartBleWatcher(sender, args);
            });
        m_watcher.Start();
    }
    int32_t Home::MyProperty()
    {  
       throw hresult_not_implemented();  
    }  

    Home::~Home()
    {
        if (m_timer)
        {
            m_timer.Stop();
            m_timer = nullptr;
        }
    }
    void Home::MyProperty(int32_t /* value */)  
    {  
       throw hresult_not_implemented();  
    }

    void Home::checkValueErrorMesenge(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&)
    {
        if (!link->errorMesenge.empty())
        {
            MainErrorBar().IsOpen(true);
            MainErrorBar().Message(link->errorMesenge);
            link->errorMesenge = L"";
        }
        if (!link->fromRobot.empty())
        {
            if (link->fromRobot.front() == L"REQUEST: Enter connection code\r\n")
            {
                PasswordScreen().Visibility(Visibility::Visible);
                LoadingScreen().Visibility(Visibility::Collapsed);
                LoadingScreen().IsHitTestVisible(false);
                PasswordScreen().IsHitTestVisible(true);
                
            }
            else if(link->fromRobot.front() == L"STATUS: Connected successfully\r\n")
            {
                DoubleAnimation fadeOut{};
                fadeOut.Duration(DurationHelper::FromTimeSpan(std::chrono::milliseconds(500)));
                fadeOut.From(1.0);
                fadeOut.To(0.0);

                Storyboard storyboardOut{};
                Storyboard::SetTarget(fadeOut, LoadingScreen());
                Storyboard::SetTargetProperty(fadeOut, L"Opacity");
                storyboardOut.Children().Append(fadeOut);

                // 2. ѕо€вление checkmarkScreen
                DoubleAnimation fadeIn{};
                fadeIn.Duration(DurationHelper::FromTimeSpan(std::chrono::milliseconds(500)));
                fadeIn.From(0.0);
                fadeIn.To(1.0);

                Storyboard storyboardIn{};
                Storyboard::SetTarget(fadeIn, CheckmarkAnimScreen());
                Storyboard::SetTargetProperty(fadeIn, L"Opacity");
                storyboardIn.Children().Append(fadeIn);
                
                // ѕри завершении первой анимации запускаем вторую
                storyboardOut.Completed([=](auto const&, auto const&) {
                    LoadingScreen().Visibility(Visibility::Collapsed);
                    LoadingScreen().IsHitTestVisible(false);

                    CheckmarkAnimScreen().Visibility(Visibility::Visible);
                    CheckmarkAnimScreen().IsHitTestVisible(true);

                    storyboardIn.Begin();

                    Player().Source(AnimatedVisuals::CheckmarkAnim());
                    auto asyncAction = Player().PlayAsync(0.5, 1, false);
                    PlaySound(L"C:\\Windows\\Media\\Windows Hardware Insert.wav", nullptr, SND_ALIAS | SND_ASYNC);

                    DoubleAnimation fadeInT{};
                    fadeInT.Duration(DurationHelper::FromTimeSpan(std::chrono::milliseconds(500)));
                    fadeInT.From(0.0);
                    fadeInT.To(1.0);

                    Storyboard storyboardInT{};
                    Storyboard::SetTarget(fadeInT, TextOk());
                    Storyboard::SetTargetProperty(fadeInT, L"Opacity");
                    storyboardInT.Children().Append(fadeInT);

                    storyboardInT.Begin();

                    asyncAction.Completed([this](auto&&, auto&&) {
                        // This will execute after first animation completes
                        Sleep(1000);
                        auto asyncAction2 = Player().PlayAsync(0, 0.5, false);
                         asyncAction2.Completed([this](auto&&, auto&&) {
                            DoubleAnimation fadeOut2{};
                            PlaySound(L"C:\\Windows\\Media\\Windows Notify System Generic.wav", nullptr, SND_ALIAS | SND_ASYNC);
                            fadeOut2.Duration(DurationHelper::FromTimeSpan(std::chrono::milliseconds(250)));
                            fadeOut2.From(1.0);
                            fadeOut2.To(0.0);

                            Storyboard storyboardOut2{};
                            Storyboard::SetTarget(fadeOut2, CheckmarkAnimScreen());
                            Storyboard::SetTargetProperty(fadeOut2, L"Opacity");
                            storyboardOut2.Children().Append(fadeOut2);
                            storyboardOut2.Begin();

                           
                            auto gridElement = MegaMainGrid().try_as<winrt::Microsoft::UI::Xaml::UIElement>();
                            if (!gridElement)
                                return;

                            auto parentFrame = find_ancestor<winrt::Microsoft::UI::Xaml::Controls::Frame>(gridElement);
                            if (parentFrame)
                            {
                                link->isConnected = true;
                                uint64_t ptr = reinterpret_cast<uint64_t>(link);
                                parentFrame.Navigate(
                                    winrt::xaml_typename<TLTC__winUi3_::MainPageApp>(),
                                    winrt::box_value(ptr),
                                     DrillInNavigationTransitionInfo()
                                );
                            }
                        });  
                    });
                });

                storyboardOut.Begin();
            }
            link->fromRobot.pop();
        }
    }

    void Home::updateElementsBluetoothDevicesComboBox()
    {
        if (bluetoothDevices.empty())
        {
            BluetoothDevicesComboBox().IsEnabled(false);
            return;
        }

        // —охран€ем текущий выбранный адрес перед обновлением
        uint64_t selectedAddress = 0;
        if (BluetoothDevicesComboBox().SelectedItem())
        {
            auto selectedItem = BluetoothDevicesComboBox().SelectedItem().as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>();
            selectedAddress = unbox_value<uint64_t>(selectedItem.Tag());
        }

        BluetoothDevicesComboBox().IsEnabled(true);

        // Ѕлокируем событи€ выбора во врем€ обновлени€

        BluetoothDevicesComboBox().Items().Clear();

        for (const auto& device : bluetoothDevices)
        {
            if (device.Address == 0)
            {
                continue;
            }

            winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem item;
            if (device.Name.empty())
            {
                wchar_t macAddressFormatted[18];
                std::swprintf(macAddressFormatted, sizeof(macAddressFormatted) / sizeof(wchar_t),
                    L"%02X:%02X:%02X:%02X:%02X:%02X",
                    (device.Address >> 40) & 0xFF,
                    (device.Address >> 32) & 0xFF,
                    (device.Address >> 24) & 0xFF,
                    (device.Address >> 16) & 0xFF,
                    (device.Address >> 8) & 0xFF,
                    (device.Address >> 0) & 0xFF
                );
                item.Content(box_value(macAddressFormatted));
            }
            else
            {
                item.Content(box_value(device.Name));
            }
            item.Tag(box_value(device.Address));
            BluetoothDevicesComboBox().Items().Append(item);

            // ¬осстанавливаем выбор, если адрес совпадает
            if (device.Address == selectedAddress)
            {
                BluetoothDevicesComboBox().SelectedItem(item);
            }
        }
    }
    void Home::StartBleWatcher(BluetoothLEAdvertisementWatcher const& sender, BluetoothLEAdvertisementReceivedEventArgs const& args)
    {
        
        std::lock_guard lock(this->g_devicesMutex);

        auto it = std::find_if(bluetoothDevices.begin(), bluetoothDevices.end(), [&](BluetoothDevice const& dev)
            {
                return dev.Address == args.BluetoothAddress();
            });

        if (it != bluetoothDevices.end())
        {
            // ”же есть такое устройство Ч ничего не делаем
            return;
        }

        // ƒобавл€ем новое устройство
        std::wstring name;
        if (!args.Advertisement().LocalName().empty())
        {
            name = args.Advertisement().LocalName().c_str();
        }

        bluetoothDevices.emplace_back(args.BluetoothAddress(), name);

        DispatcherQueue().TryEnqueue([this]()
            {
                updateElementsBluetoothDevicesComboBox();
         });

    }
    void Home::StopBleWatcher()
    {
        if (m_watcher)
        {
            m_watcher.Stop();
        }
    }

}
void winrt::TLTC__winUi3_::implementation::Home::DigitBox_TextChanged(IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const&)
{
    auto box = sender.as<winrt::Microsoft::UI::Xaml::Controls::TextBox>();
    if (box.Text().size() == 1)
    {
        if (box == DigitBox1()) DigitBox2().Focus(winrt::Microsoft::UI::Xaml::FocusState::Programmatic);
        else if (box == DigitBox2()) DigitBox3().Focus(winrt::Microsoft::UI::Xaml::FocusState::Programmatic);
        else if (box == DigitBox3()) DigitBox4().Focus(winrt::Microsoft::UI::Xaml::FocusState::Programmatic);
        else if (box == DigitBox4()) ContinueButton().Focus(winrt::Microsoft::UI::Xaml::FocusState::Programmatic);
    }
}
void winrt::TLTC__winUi3_::implementation::Home::SelectButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{  
   if (BluetoothDevicesComboBox().Items().Size() == 0)  
   {  
       MainInfoBar().Message(L"”стройства не найдены, ожидайте");  
       MainInfoBar().IsOpen(true);  
   }  
   else if (BluetoothDevicesComboBox().SelectedItem() == nullptr)  
   {  
       MainInfoBar().Message(L"¬ы не выбрали ваше устройство");  
       MainInfoBar().IsOpen(true);  
   }  
   else  
   {  
       Progess().IsIndeterminate(false);
       MainContent().IsHitTestVisible(false);
       Progess().IsHitTestVisible(true);
       Progess().Visibility(Visibility::Collapsed);  
       StopBleWatcher();  
       LoadingScreen().Visibility(Visibility::Visible);  
       auto storyboard = FadeOutStoryboard();  
       storyboard.Begin();  
       auto storyboard2 = FadeInStoryboard();  
       storyboard.Begin();  

       uint64_t address = 0;
       try
       {
           if (auto item = BluetoothDevicesComboBox().SelectedItem().try_as<winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem>())
           {
               if (auto tag = item.Tag())
               {
                   address = unbox_value<uint64_t>(tag);
               }
           }
       }
       catch (...)
       {
       }

       char macAddressFormatted[18];
       std::snprintf(macAddressFormatted, sizeof(macAddressFormatted) / sizeof(char),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           (address >> 40) & 0xFF,
           (address >> 32) & 0xFF,
           (address >> 24) & 0xFF,
           (address >> 16) & 0xFF,
           (address >> 8) & 0xFF,
           (address >> 0) & 0xFF
       );
       link = new LinkWithRobot(std::string(macAddressFormatted));
   }  
}

void winrt::TLTC__winUi3_::implementation::Home::LoadingScreen_LayoutUpdated(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e)
{
    if (LoadingScreen().Visibility() == winrt::Microsoft::UI::Xaml::Visibility::Visible)
    {
        if (!m_timer)
        {
            m_timer = winrt::Microsoft::UI::Xaml::DispatcherTimer();
            m_timer.Interval(std::chrono::milliseconds(100)); // 100 мс дл€ проверки
            m_timer.Tick({ this, &Home::checkValueErrorMesenge });
        }
        m_timer.Start();
    }
    else
    {
        if (m_timer)
        {
            m_timer.Stop();
        }
    }
}

void winrt::TLTC__winUi3_::implementation::Home::ContinueButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
{
    link->sendMesenge(winrt::to_string(DigitBox1().Text() + DigitBox2().Text() + DigitBox3().Text() + DigitBox4().Text()));
    LoadingScreen().Visibility(Visibility::Visible);
    PasswordScreen().Visibility(Visibility::Collapsed);
    PasswordScreen().IsHitTestVisible(false);
    LoadingScreen().IsHitTestVisible(true);
}