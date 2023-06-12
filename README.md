### hid.hpp
Single header C++23 wrapper for [libusb/hidapi](https://github.com/libusb/hidapi).

### Building the tests
- Run `bootstrap.[bat|sh]`. This will install doctest + hidapi, and create the project under the `./build` directory.
- Run cmake on the `./build` directory and toggle `BUILD_TESTS`.
- Configure, generate, make.

### Using
The cmake project exports the `hid::hid` target, hence you can:
```cmake
find_package         (hid CONFIG REQUIRED)
target_link_libraries([YOUR_PROJECT_NAME] PRIVATE hid::hid)
```
And then point the `hid_DIR` to the build or installation directory.
Alternatively, you can just copy `include/hid/hid.hpp` to your project.

### Example usage
```cpp
#include <hid/hid.hpp>
#include <iostream>

int main(int argc, char** argv)
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

  return 0;
}
```
See the tests for more.

### Design decisions
- **Enums**:
  - `hid_bus_type`    is wrapped to `hid::bus_type` which is a strongly-typed enum.
- **Structs**:
  - `hid_device_info` is wrapped to `hid::device_info` which contains (wide) strings instead of (wide) char arrays.
  - `hid_api_version` is aliased to `hid::api_version`.
- **Functions**:
  - **Free functions**:
    | C Function Signature                                                  | C++ Function Signature                                                                                                                    | Comments                                                                                                                                                                                                                                                                                                                               |
    |-----------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
    | `hid_error (nullptr) -> const wchar_t*`                               | `hid::error () -> std::wstring`                                                                                                           | Free function for the null device case. Does not return an `std::expected<...>` since it is guaranteed to not fail.                                                                                                                                                                                                                    |
    | `hid_init () -> int`                                                  | `hid::init () -> std::expected<void, std::wstring>`                                                                                       | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | `hid_exit () -> int`                                                  | `hid::exit () -> std::expected<void, std::wstring>`                                                                                       | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | `hid_enumerate (uint16_t, uint16_t) -> hid_device_info*`              | `hid::enumerate (std::uint16_t = 0, std::uint16_t = 0) -> std::vector <hid::device_info>`                                                 | Copies the `hid_device_info*` linked list to a `std::vector<hid::device_info>` and frees it. Does not return an `std::expected<...>` since a null result does not necessarily imply a failure, but no matching devices. The user is responsible for calling `hid::error()` manually for checking the reason. Open to better solutions. |
    | `hid_free_enumeration (hid_device_info*) -> void`                     | N/A                                                                                                                                       | Called internally within `hid::enumerate(...)`.                                                                                                                                                                                                                                                                                        |
    | `hid_open_path (const char*) -> hid_device*`                          | `hid::open (const std::string&) -> std::expected<hid::device, std::wstring>`                                                              | Operator overloading enables using the same name for the two functions. The unexpected state contains the result of `hid::error()`. The reason to use a free function rather than the `hid::device` constructors is that the constructors can not transmit errors unless they throw (which this library avoids).                       |
    | `hid_open (uint16_t, uint16_t, const wchar_t*) -> hid_device*`        | `hid::open (std::uint16_t, std::uint16_t, const std::optional<std::wstring>& = std::nullopt) -> std::expected<hid::device, std::wstring>` | The unexpected state contains the result of `hid::error()`. The reason to use a free function rather than the `hid::device` constructors is that the constructors can not transmit errors unless they throw (which this library avoids).                                                                                               |
    | N/A                                                                   | `hid::open (const hid::device_info&) -> std::expected<hid::device, std::wstring>`                                                         | Convenience for `hid::open(device_info.path)` and (if that fails) for `hid::open(device_info.vendor_id, device_info.product_id, device_info.serial_number)`.                                                                                                                                                                           |
    | `hid_version () -> const hid_api_version*`                            | `hid::version () -> const hid::api_version&`                                                                                              | Does not return an `std::expected<...>` since it is guaranteed to not fail.                                                                                                                                                                                                                                                            |
    | `hid_version_str () -> const char*`                                   | `hid::version_str () -> std::string`                                                                                                      | Does not return an `std::expected<...>` since it is guaranteed to not fail.                                                                                                                                                                                                                                                            |
  - **Device functions**: Functions accepting a `hid_device*` as their first parameter are members of the `hid::device` class which encapsulates a `hid_device*`.
    | C Function Signature                                                  | C++ Function Signature                                                                                                                    | Comments                                                                                                                                                                                                                                                                                                                               |
    |-----------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
    | `hid_close (hid_device*) -> void`                                     | `hid::device::~device ()`                                                                                                                 | Destructor closes the device on scope exit.                                                                                                                                                                                                                                                                                            |
    | N/A                                                                   | `hid::device::native () -> const hid_device*`                                                                                             | Accessor to native hid_device*.                                                                                                                                                                                                                                                                                                        |
    | `hid_get_device_info (hid_device*) -> hid_device_info*`               | `hid::device::device_info () -> std::expected<device_info, std::wstring>`                                                                 | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | `hid_get_serial_number_string (hid_device*, wchar_t*, size_t) -> int` | `hid::device::serial_number (const std::size_t = 256) -> std::expected<std::wstring, std::wstring>`                                       | The unexpected state contains the result of `hid::error()`. Default maximum length is 256.                                                                                                                                                                                                                                             |
    | `hid_get_manufacturer_string (hid_device*, wchar_t*, size_t) -> int`  | `hid::device::manufacturer_string (const std::size_t = 256) -> std::expected<std::wstring, std::wstring>`                                 | The unexpected state contains the result of `hid::error()`. Default maximum length is 256.                                                                                                                                                                                                                                             |
    | `hid_get_product_string (hid_device*, wchar_t*, size_t) -> int`       | `hid::device::product_string (const std::size_t = 256) -> std::expected<std::wstring, std::wstring>`                                      | The unexpected state contains the result of `hid::error()`. Default maximum length is 256.                                                                                                                                                                                                                                             |
    | `hid_get_indexed_string (hid_device*, int, wchar_t*, size_t) -> int`  | `hid::device::indexed_string (const std::int32_t, const std::size_t = 256) -> std::expected<std::wstring, std::wstring>`                  | The unexpected state contains the result of `hid::error()`. Default maximum length is 256.                                                                                                                                                                                                                                             |
    | ``                                                                    | ``                                                                                                                                        | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | ``                                                                    | ``                                                                                                                                        | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | ``                                                                    | ``                                                                                                                                        | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | ``                                                                    | ``                                                                                                                                        | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | ``                                                                    | ``                                                                                                                                        | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | ``                                                                    | ``                                                                                                                                        | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | ``                                                                    | ``                                                                                                                                        | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | ``                                                                    | ``                                                                                                                                        | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | `hid_error (hid_device*) -> const wchar_t*`                           | `hid::device::error () -> std::wstring`                                                                                                   | Member function for the non-null device case. Does not return an `std::expected<...>` since it is guaranteed to not fail.                                                                                                                                                                                                              |

### Future direction
- Platform-specific function wrappers.
- Consider casting/accepting output/input byte arrays to/as arbitrary types.
- Interacting with binary data could be eased through mapping text descriptors to binary descriptors and vice versa. This could also be used to determine and set the exact sizes of the reports without the user. See [Frank Zhao's online parser](https://eleccelerator.com/usbdescreqparser/) as an example.