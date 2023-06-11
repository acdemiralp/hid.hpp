#include <doctest/doctest.h>
#include <hid/hid.hpp>
#include <iostream>

TEST_CASE("HID Test")
{
  hid::init();

  const auto device_infos = hid::enumerate();
  for (auto& device_info : device_infos)
    std::wcout << device_info << "\n";

  if (!device_infos.empty())
  {
    {
      // Open a device by vendor and product ID.
      const auto device = hid::open(device_infos[0].vendor_id, device_infos[0].product_id);
    }
    {
      // Open a device by path.
      const auto device = hid::open(device_infos[0].path);
    }
    {
      // Open a device by device info (by device_info.path and if that fails, by device_info.vendor_id and device_info.product_id).
      const auto device = hid::open(device_infos[0]);

      // Error handling can be bypassed if the results are assumed to be valid:
      std::wcout << "device_info: "                                           << "\n";
      std::wcout << *device->device_info()                                    << "\n";
      std::wcout << "serial_number: "       << *device->serial_number      () << "\n";
      std::wcout << "manufacturer_string: " << *device->manufacturer_string() << "\n";
      std::wcout << "product_string: "      << *device->product_string     () << "\n";

      // Error handling can be performed if the results are not assumed to be valid:
      if (auto result = device->indexed_string(1))
        std::wcout << "indexed_string(1): " << result.value() << "\n";
      else
        std::wcout << "Error: "             << result.error() << "\n";

      const auto report_descriptor = device->report_descriptor();
      std::wcout << "report_descriptor: ";
      for (const auto value : *report_descriptor)
        std::wcout << std::hex << std::uppercase << value << " ";
      std::wcout << "\n";

      const auto input_report_0 = device->input_report(0);
      std::wcout << "input_report(0): ";
      for (const auto value : *input_report_0)
        std::wcout << std::hex << std::uppercase << value << " ";
      std::wcout << "\n";

      const auto feature_report_0 = device->feature_report(0);
      std::wcout << "feature_report(0): ";
      for (const auto value : *feature_report_0)
        std::wcout << std::hex << std::uppercase << value << " ";
      std::wcout << "\n";

      // TODO: read, read_timeout, send_feature_report (?), write (?).

      device->set_nonblocking(true);
    }
  }

  std::cout << "Version: " << 
    hid::version()->major << " " << 
    hid::version()->minor << " " << 
    hid::version()->patch << "\n";
  std::cout << "Version String: " << hid::version_str() << "\n";

  hid::exit();
}