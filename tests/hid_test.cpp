#include <doctest/doctest.h>
#include <hid/hid.hpp>

#include <iostream>

TEST_CASE("HID Test")
{
  // hid::init();

  // std::cout << "Version: " << hid::version_str() << "\n";

  auto device_infos = hid::enumerate();
  for (auto& device_info : device_infos)
    std::cout << "Device:\n" << 
      " " << device_info.path                                                                            << "\n" <<
      " " << device_info.vendor_id                                                                       << "\n" <<
      " " << device_info.product_id                                                                      << "\n" <<
      " " << std::string(device_info.serial_number      .begin(), device_info.serial_number      .end()) << "\n" <<
      " " << device_info.release_number                                                                  << "\n" <<
      " " << std::string(device_info.manufacturer_string.begin(), device_info.manufacturer_string.end()) << "\n" <<
      " " << std::string(device_info.product_string     .begin(), device_info.product_string     .end()) << "\n" <<
      " " << device_info.usage_page                                                                      << "\n" <<
      " " << device_info.usage                                                                           << "\n" <<
      " " << device_info.interface_number                                                                << "\n" <<
      " " << static_cast<std::int32_t>(device_info.bus_type)                                             << "\n";

  auto device = hid::open(device_infos[0]);

  // hid::exit();
}