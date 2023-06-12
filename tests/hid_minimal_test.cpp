#include <doctest/doctest.h>
#include <hid/hid.hpp>
#include <iostream>

TEST_CASE("HID Minimal Test")
{
  // Enumerate and print devices.
  const auto device_infos = hid::enumerate();
  for (const auto& device_info : device_infos)
    std::wcout << device_info << "\n";

  if (!device_infos.empty())
  {
    // Open the first device.
    const auto device = hid::open(device_infos[0]);
    if (device)
    {
      // Print its report descriptor.
      const auto descriptor = device->report_descriptor();
      std::wcout << "report_descriptor: ";
      for (const auto value : *descriptor)
        std::wcout << std::format(L"{:#04x}", value) << " ";
      std::wcout << "\n";

      // Set it to non-blocking.
      device->set_nonblocking(true);

      // Read from it.
      if (const auto result = device->read())
      {
        std::wcout << "read: ";
        for (const auto value : *result)
          std::wcout << std::format(L"{:#04x}", value) << " ";
        std::wcout << "\n";
      }
      else
        std::wcout << "read failed with error: " << result.error() << "\n";

      // Write to it.
      std::vector<std::uint8_t> data(8, 0x00);
      if (const auto result = device->write(data))
        std::wcout << "wrote " << *result << " bytes.\n";
      else
        std::wcout << "write failed with error: " << result.error() << "\n";
    }
    else
      std::wcout << "open failed with error: " << device.error() << "\n";
  }
}