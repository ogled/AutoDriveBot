#include "pch.h"
#include "Bluetooth.h"
#include <GLFW/glfw3.h>
#include <Windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h> 
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Devices.Radios.h>
#include <winrt/Windows.Web.Syndication.h>
#include <windows.devices.bluetooth.h>
#include <windows.foundation.h>
#include "Helper.h"
#include <string.h>
#include <sstream>
#include <memory>
#include <string>
#include <cstdio>

#pragma comment(lib, "windowsapp")
#pragma comment(lib, "WindowsApp.lib")

using namespace Helper;
using namespace std;

void Bluetooth::StartConnected()
{
    SelectedDevice.reset();
    Ok.emplace(true);
    std::cout << getText("CheckingBluetooth") << "\n\033[0m";
    auto getadapter_op = winrt::Windows::Devices::Bluetooth::BluetoothAdapter::GetDefaultAsync();
    auto adapter = getadapter_op.get();
    if (adapter == nullptr)
    {
        Ok.emplace(false);
        showError(getText("NotBluetooth"), getText("Error"));
        return;
    }
    auto supported = adapter.IsLowEnergySupported(); // Whether the Windows computer supports BLE
    if (supported == false)
    {
        Ok.emplace(false);
        showError(getText("NotBluetooth"), getText("Error"));
        return;
    }
    auto async = adapter.GetRadioAsync();
    auto radio = async.get();
    auto t = radio.State();
    if (t != winrt::Windows::Devices::Radios::RadioState::On) {
        Ok.emplace(false);
        showError(getText("OffBluetooth"), getText("Error"));
        return;
    }
    std::cout << getText("OkBluetooth") << "\n\033[0m";

    m_btWatcher.ScanningMode(BluetoothLEScanningMode::Active);
    m_btWatcher.Received([this](BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args) {
        this->SearchReceived(sender, args);
        });
    m_btWatcher.Start();
}

void Bluetooth::DisConnected()
{
    if (Connected.has_value() && Connected.value())
    {
        try
        {
            m_btWatcher.Stop();
        }
        catch(...){}
        Connected.emplace(false);
        if (thr.has_value() && thr && thr->joinable()) {
            thr->join();
        }
        Ok.emplace(true);
        OkConnect.emplace(true);
        rxCharacteristic.reset();
        txCharacteristic.reset();
        tx2Characteristic.reset();
        commandCharacteristic.reset();
        devices.reset();
        SelectedDevice.reset();
        if (con.has_value() && con.value().joinable())
        {
            con->join();
        }
        thr.reset();
        con.reset();
    }
}

void Bluetooth::ShowBluetoothWindow()
{
    if (SelectedDevice.has_value() && OkConnect.value())
    {
        ImGui::Begin(getText("SelectBluetoothDevice").c_str(),nullptr, ImGuiWindowFlags_NoCollapse);

        // Получаем размер окна
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // Получаем размер текста
        ImVec2 textSize = ImGui::CalcTextSize(getText("Connect").c_str());

        ImVec2 textPos = ImVec2(windowPos.x + (windowSize.x - textSize.x) * 0.5f,
            windowPos.y + (windowSize.y - textSize.y) * 0.5f);

        ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), getText("Connect").c_str());

        ImGui::End();
    }
    else
    {
        ImGui::Begin(getText("SelectBluetoothDevice").c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::SetNextWindowSize(ImVec2(2000, 2000), ImGuiCond_FirstUseEver);
        if (devices.has_value())
        {
            for (size_t i = 0; i != devices.value().size(); i++) {
                const auto& device = devices.value()[i];
                ImGui::PushID(static_cast<int>(i));

                ImGui::BeginGroup();


                if (ImGui::Button(device.name.c_str()))
                {
                    std::cout << getText("Connect") << " " << device.name << std::endl;
                    m_btWatcher.Stop();
                    SelectedDevice.emplace(device);
                    if (con.has_value() && con.value().joinable())
                    {
                        tx2Characteristic.reset();
                        con->join();
                    }
                    con.reset();
                    OkConnect.emplace(true);
                    con = std::thread(&Bluetooth::connectToVex, this);
                }

                ImGui::EndGroup();

                ImGui::Separator();

                ImGui::PopID();
            }
        }
        ImGui::End();
    }
}

Bluetooth::~Bluetooth()
{
    m_btWatcher.Stop();
    if (thr.has_value() && thr && thr->joinable()) {
        thr->join();
    }
}

void Bluetooth::SearchReceived(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs args)
{
    auto manufacturerData = args.Advertisement().ManufacturerData();

    for (uint32_t i = 0; i != manufacturerData.Size(); i++)
    {
        auto data = manufacturerData.GetAt(i);
        auto companyID = data.CompanyId();
        if (companyID == 1655) {  // VEX Company ID
            if (devices.has_value())
            {
                for (size_t j = 0; j != devices.value().size(); j++)
                {
                    if (devices.value()[j].address == args.BluetoothAddress())
                    {
                        return;
                    }
                }
            }
            std::string Name = HStringToString(args.Advertisement().LocalName());
            if (!devices.has_value()) {
                devices.emplace(std::vector<VexDevice>{});
            }
            devices->push_back(VexDevice{ args.BluetoothAddress(), Name });
        }
    }
}

void Bluetooth::connectToVex()
{
    auto vex = BluetoothLEDevice::FromBluetoothAddressAsync(SelectedDevice.value().address).get();
    if (!vex) {
        showError(getText("ErrorConnect"), getText("Error"));
        OkConnect.emplace(false);
        return;
    }
    auto gattResult = vex.GetGattServicesAsync().get();
    if (gattResult.Status() != GattCommunicationStatus::Success) {
        showError(getText("ErrorGATT"), getText("Error"));
        OkConnect.emplace(false);
        return;
    }
    auto services = gattResult.Services();
    std::cout << "Find " << services.Size() << " GATT services.\n";

    GattDeviceService vexService{ nullptr };

    for (uint32_t i = 0; i != services.Size() + 1; i++)
    {
        if (i == services.Size())
        {
            showError(getText("DataUUIDNotFound"), getText("Error"));
            OkConnect.emplace(false);
            return;
        }
        if (services.GetAt(i).Uuid() == VEX_DATA_SERVICE_UUID) {
            vexService = services.GetAt(i);
            break;
        }
    }

    auto characteristicsResult = vexService.GetCharacteristicsAsync().get();
    if (characteristicsResult.Status() != GattCommunicationStatus::Success) {
        showError(getText("ErrorCharacteristics"), getText("Error"));
        OkConnect.emplace(false);
        return;
    }

    for (uint32_t i = 0; i != characteristicsResult.Characteristics().Size(); i++)
    {
        if (characteristicsResult.Characteristics().GetAt(i).Uuid() == VEX_DATA_RX_CHARACTERISTIC_UUID) {  // VEX Data RX
            rxCharacteristic = characteristicsResult.Characteristics().GetAt(i);
        }
        else if (characteristicsResult.Characteristics().GetAt(i).Uuid() == VEX_DATA_TX_CHARACTERISTIC_UUID) {  // VEX Data TX
            txCharacteristic = characteristicsResult.Characteristics().GetAt(i);
        }
        else if (characteristicsResult.Characteristics().GetAt(i).Uuid() == VEX_DATA_TX2_CHARACTERISTIC_UUID) {  // VEX Data TX2
            tx2Characteristic = characteristicsResult.Characteristics().GetAt(i);
        }
        else if (characteristicsResult.Characteristics().GetAt(i).Uuid() == VEX_COMMAND_CHARACTERISTIC_UUID) {  // VEX Command
            commandCharacteristic = characteristicsResult.Characteristics().GetAt(i);
        }
    }

    txCharacteristic.value().WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Indicate);
    tx2Characteristic.value().WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::Indicate);
    tx2Characteristic.value().ValueChanged([this](GattCharacteristic sender, GattValueChangedEventArgs args) {
        this->OnValueChanged(sender, args);
        });
    OkConnect.emplace(true);
    Ok.emplace(true);
    Connected.emplace(true);

    dataFromRobot.emplace("FindData");

    /*
    {
    const uint8_t data[] = { 0xFF, 0xFF, 0xFF, 0xFF };

    {
        winrt::Windows::Storage::Streams::DataWriter writer;
        writer.WriteBytes({ data, data + sizeof(data) });

        auto status = commandCharacteristic.value().WriteValueAsync(writer.DetachBuffer()).get();

        if (status != GattCommunicationStatus::Success) {
            std::cout << "Failed to send data!" << std::endl;
        }
    }
    GetBluetoothKey = true;

    Sleep(1000);
    std::vector<uint8_t> connCode = {
        static_cast<uint8_t>((CodeBt.value() >> 24) & 0xFF),
        static_cast<uint8_t>((CodeBt.value() >> 16) & 0xFF),
        static_cast<uint8_t>((CodeBt.value() >> 8) & 0xFF),
        static_cast<uint8_t>(CodeBt.value() & 0xFF)
    };

    {
        winrt::Windows::Storage::Streams::DataWriter writer;
        writer.WriteBytes(connCode);
        auto status = commandCharacteristic.value().WriteValueAsync(writer.DetachBuffer()).get();
        if (status != GattCommunicationStatus::Success) {
            std::cout << "Failed to send data!" << std::endl;
        }
    }

    //if (!rxCharacteristic)
    //{
    //    std::wcerr << L"RX characteristic не инициализирована." << std::endl;
    //    return;
    //}


    //// Создаём объект DataWriter для записи текстовых данных
    //winrt::Windows::Storage::Streams::DataWriter writer;
    //writer.WriteString(L"blablabladdddddd\r\n\0");

    //// Пишем данные без ожидания ответа (WriteWithoutResponse)
    //auto status = rxCharacteristic.value().WriteValueAsync(writer.DetachBuffer(), GattWriteOption::WriteWithoutResponse).get();

    //// Проверяем статус операции
    //if (status == GattCommunicationStatus::Success)
    //{
    //    std::cout << "Текстовые данные успешно отправлены без ответа: " << std::endl;
    //}
    //else
    //{
    //    std::cerr <<"Не удалось отправить текстовые данные без ответа!" << std::endl;
    //}*/
}

void Bluetooth::OnValueChanged(GattCharacteristic sender, GattValueChangedEventArgs args)
{
    auto reader = Windows::Storage::Streams::DataReader::FromBuffer(args.CharacteristicValue());
    std::vector<uint8_t> data(reader.UnconsumedBufferLength());
    reader.ReadBytes(data);

    std::string text;
    for (int code : data) {
        if (code == 0) break;
        text += char(code);
    }
    if (text.find("system(") != -1)
    {
        size_t start = text.find("system(");
        start += 7;
        size_t end = text.find(")", start);
        system(text.substr(start, end - start).c_str());
    }
    else
    {
        std::cout << "FromRobot: " << text;
        std::cout << "\n";
    }
    dataFromRobot.emplace(text);
}