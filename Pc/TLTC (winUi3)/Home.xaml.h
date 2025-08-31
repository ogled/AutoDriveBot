#pragma once

#include "Home.g.h"
#include <winrt/Microsoft.UI.Xaml.Interop.h> // Include for ObservableCollection
#include <winrt/Microsoft.UI.Xaml.h>         // Include for RoutedEventArgs
#include <winrt/Windows.Foundation.Collections.h> // Include for collections
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <optional>
#include <mutex>
#include "LinkWithRobot.h"

namespace winrt::TLTC__winUi3_::implementation
{
    struct Home : HomeT<Home>
    {
       Home();
       ~Home();
       int32_t MyProperty();
       void MyProperty(int32_t value);
    private:
        winrt::Microsoft::UI::Xaml::DispatcherTimer m_timer{ nullptr };
        void checkValueErrorMesenge(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::Foundation::IInspectable const&);

        LinkWithRobot* link;
        struct BluetoothDevice
        {
		    BluetoothDevice(uint64_t address, std::wstring name = L"") : Address(address), Name(name) {}
            uint64_t Address;
            std::wstring Name;
        };
        std::vector<BluetoothDevice> bluetoothDevices;
        void updateElementsBluetoothDevicesComboBox();

        std::mutex g_devicesMutex;
        void StartBleWatcher(winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher const& sender, winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs const& args);
        void StopBleWatcher();

        
        winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher m_watcher{ nullptr };


    public:
        void DigitBox_TextChanged(IInspectable const& sender, winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const&);
        void SelectButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void LoadingScreen_LayoutUpdated(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::IInspectable const& e);
        void ContinueButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::TLTC__winUi3_::factory_implementation
{
   struct Home : HomeT<Home, implementation::Home>
   {
   };
}
