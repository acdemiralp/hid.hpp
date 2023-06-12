#include <doctest/doctest.h>
#include <hid/hid.hpp>
#include <iostream>

TEST_CASE("HID Test")
{
  std::wcout << std::wstring(80, '#')                               << "\n";
  std::wcout << "version: "           << hid::version()             << "\n";
  std::wcout << "version_str: "       << hid::version_str().c_str() << "\n";
  std::wcout << std::wstring(80, '#')                               << "\n";

  if (const auto result = hid::init(); !result)
  {
    std::wcout << "hid::init() failed with error: " << result.error() << "\n";
    return;
  }

  const auto device_infos = hid::enumerate();
  if (device_infos.empty())
    std::wcout << "hid::enumerate() failed with (potential) error: " << hid::error() << "\n";
  else
  {
    for (const auto& device_info : device_infos)
      std::wcout << device_info << "\n";
    std::wcout << std::wstring(80, '#') << "\n";

    {
      // Open a device by vendor and product ID (closes on scope exit).
      const auto device = hid::open(device_infos.front().vendor_id, device_infos.front().product_id);
      if (!device)
        std::wcout << "hid::open(vendor_id, product_id) failed with error: " << device.error() << "\n";
    }
    {
      // Open a device by path (closes on scope exit).
      const auto device = hid::open(device_infos.front().path);
      if (!device)
        std::wcout << "hid::open(path) failed with error: " << device.error() << "\n";
    }
    {
      // Open a device by device info (by device_info.path and if that fails, by device_info.vendor_id and device_info.product_id).
      const auto device = hid::open(device_infos.front());
      if (!device)
        std::wcout << "hid::open(device_info) failed with error: " << device.error() << "\n";
      else
      {
        std::wcout << "opened device at path: " << device_infos.front().path.c_str() << "\n\n";

        if (const auto result = device->device_info())
          std::wcout << "device_info: " << "\n" << *result << "\n";
        else
          std::wcout << "hid::device::device_info() failed with error: " << result.error() << "\n";

        if (const auto result = device->serial_number())
          std::wcout << "serial_number: " << *result << "\n";
        else
          std::wcout << "hid::device::serial_number() failed with error: " << result.error() << "\n";

        if (const auto result = device->manufacturer_string())
          std::wcout << "manufacturer_string: " << *result << "\n";
        else
          std::wcout << "hid::device::manufacturer_string() failed with error: " << result.error() << "\n";

        if (const auto result = device->product_string())
          std::wcout << "product_string: " << *result << "\n";
        else
          std::wcout << "hid::device::product_string() failed with error: " << result.error() << "\n";

        std::wcout << "\n";

        for (auto i = 0; i < 4; ++i)
          if (const auto result = device->indexed_string(i))
            std::wcout << "indexed_string(" << i << "): " << *result << "\n";
          else
            std::wcout << "hid::device::indexed_string(" << i << ") failed with error: " << result.error() << "\n";

        std::wcout << "\n";

        if (const auto result = device->report_descriptor())
        {
          std::wcout << "report_descriptor: ";
          for (const auto value : *result)
            std::wcout << std::format(L"{:#04x}", value) << " ";
          std::wcout << "\n";
        }
        else
          std::wcout << "hid::device::report_descriptor() failed with error: " << result.error() << "\n";

        std::wcout << "\n";

        for (std::uint8_t i = 0; i < 4; ++i)
          if (const auto result = device->input_report(i))
          {
            std::wcout << "input_report(" << i << "): ";
            for (const auto value : *result)
              std::wcout << std::format(L"{:#04x}", value) << " ";
            std::wcout << "\n";
          }
          else
            std::wcout << "hid::device::input_report(" << i << ") failed with error: " << result.error() << "\n";

        std::wcout << "\n";

        for (std::uint8_t i = 0; i < 4; ++i)
          if (const auto result = device->feature_report(i))
          {
            std::wcout << "feature_report(" << i << "): ";
            for (const auto value : *result)
              std::wcout << std::format(L"{:#04x}", value) << " ";
            std::wcout << "\n";
          }
          else
            std::wcout << "hid::device::feature_report(" << i << ") failed with error: " << result.error() << "\n";

        std::wcout << "\n";

        if (const auto result = device->set_nonblocking(true); !result)
          std::wcout << "hid::device::set_nonblocking(bool) failed with error: " << result.error() << "\n";

        if (const auto result = device->read())
        {
          std::wcout << "read: ";
          for (const auto value : *result)
            std::wcout << std::format(L"{:#04x}", value) << " ";
          std::wcout << "\n";
        }
        else
          std::wcout << "hid::device::read() failed with error: " << result.error() << "\n";

        if (const auto result = device->read(std::chrono::seconds(3)))
        {
          std::wcout << "read: ";
          for (const auto value : *result)
            std::wcout << std::format(L"{:#04x}", value) << " ";
          std::wcout << "\n";
        }
        else
          std::wcout << "hid::device::read(timeout) failed with error: " << result.error() << "\n";

        std::vector<std::uint8_t> data(8, 0x00);
        if (const auto result = device->write(data))
          std::wcout << "wrote " << *result << " bytes.\n";
        else
          std::wcout << "hid::device::write() failed with error: " << result.error() << "\n";

        std::vector<std::uint8_t> feature_data(8, 0x00);
        if (const auto result = device->send_feature_report(feature_data))
          std::wcout << "sent feature report (" << *result << " bytes).\n";
        else
          std::wcout << "hid::device::send_feature_report() failed with error: " << result.error() << "\n";

        std::wcout << std::wstring(80, '#') << "\n";
      }
    }
  }

  if (const auto result = hid::exit(); !result)
  {
    std::wcout << "hid::exit() failed with error: " << result.error() << "\n";
    return;
  }
}