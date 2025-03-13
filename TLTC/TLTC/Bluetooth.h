#pragma once
#include <thread>
#include <atomic>
#include <iostream>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h> 
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Radios.h>
#include <winrt/Windows.Web.Syndication.h>
#include <windows.devices.bluetooth.h>
#include <windows.foundation.h>
#include <optional>


using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::Advertisement;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace winrt::Windows::Devices::Radios;
using namespace Windows::Foundation;

class Bluetooth
{
public:
    std::optional<bool> Ok = true;
    std::optional<bool> OkConnect = true;
    std::optional<bool> Connected = false;
    std::optional<bool> GetBluetoothKey = false;
    std::optional<int> CodeBt = 4905;
	std::optional<std::string> dataFromRobot = "PleaseConnectRobot";
    void StartConnected();
    void DisConnected();
    std::optional<std::thread> thr;
    std::optional<std::thread> con;
	void ShowBluetoothWindow();
    ~Bluetooth();
private:


    void SearchReceived(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args);
    const winrt::guid VEX_DATA_SERVICE_UUID = winrt::guid(L"08590F7E-DB05-467E-8757-72F6FAEB13D5");
    const winrt::guid VEX_DATA_RX_CHARACTERISTIC_UUID = winrt::guid(L"08590f7e-db05-467e-8757-72f6faeb1326");
    const winrt::guid VEX_DATA_TX_CHARACTERISTIC_UUID = winrt::guid(L"08590F7E-DB05-467E-8757-72F6FAEB1306");
    const winrt::guid VEX_DATA_TX2_CHARACTERISTIC_UUID = winrt::guid(L"08590F7E-DB05-467E-8757-72F6FAEB1316");
    const winrt::guid VEX_COMMAND_CHARACTERISTIC_UUID = winrt::guid(L"08590F7E-DB05-467E-8757-72F6FAEB13E5");


    std::optional<GattCharacteristic> rxCharacteristic;
    std::optional<GattCharacteristic> txCharacteristic;
    std::optional<GattCharacteristic> tx2Characteristic;
    std::optional<GattCharacteristic> commandCharacteristic;
    std::vector<uint8_t> connCode;

    BluetoothLEAdvertisementWatcher m_btWatcher;
    struct VexDevice {
        uint64_t address;
        std::string name;
    };
	std::optional<std::vector<VexDevice>> devices;
    std::optional<VexDevice> SelectedDevice;

	void connectToVex();
    void OnValueChanged(GattCharacteristic sender, GattValueChangedEventArgs args);
};