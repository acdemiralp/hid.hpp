### hid.hpp
Single header C++23 wrapper for [libusb/hidapi](https://github.com/libusb/hidapi).

### Building the test
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
// TODO
```

### Design decisions
- **Enums**:
  - `hid_bus_type`    is wrapped to `hid::bus_type` which is a strongly-typed enum.
- **Structs**:
  - `hid_device_info` is wrapped to `hid::device_info` which contains (wide) strings instead of (wide) char arrays.
  - `hid_api_version` is aliased to `hid::api_version`.
- **Functions**:
  - **Free functions**:
    | C Function Signature                                          | C++ Function Signature                                                                                                                   | Comments                                                                                                                                                                                                                                                                                                                               |
    |---------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
    | `hid_error(nullptr) -> const wchar_t*`                        | `hid::error() -> std::wstring`                                                                                                           | Free function for the null device case. Does not return an `std::expected<...>` since it is guaranteed to not fail.                                                                                                                                                                                                                    |
    | `hid_version() -> const hid_api_version*`                     | `hid::version() -> const hid::api_version*`                                                                                              | Does not return an `std::expected<...>` since it is guaranteed to not fail.                                                                                                                                                                                                                                                            |
    | `hid_version_str() -> const char*`                            | `hid::version_str() -> std::string`                                                                                                      | Does not return an `std::expected<...>` since it is guaranteed to not fail.                                                                                                                                                                                                                                                            |
    | `hid_init() -> int`                                           | `hid::init() -> std::expected<void, std::wstring>`                                                                                       | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | `hid_exit() -> int`                                           | `hid::exit() -> std::expected<void, std::wstring>`                                                                                       | The unexpected state contains the result of `hid::error()`.                                                                                                                                                                                                                                                                            |
    | `hid_enumerate(uint16_t, uint16_t) -> hid_device_info*`       | `hid::enumerate(std::uint16_t = 0, std::uint16_t = 0) -> std::vector<hid::device_info>`                                                  | Copies the `hid_device_info*` linked list to a `std::vector<hid::device_info>` and frees it. Does not return an `std::expected<...>` since a null result does not necessarily imply a failure, but no matching devices. The user is responsible for calling `hid::error()` manually for checking the reason. Open to better solutions. |
    | `hid_free_enumeration(hid_device_info*) -> void`              | N/A                                                                                                                                      | Called internally within `hid::enumerate(...)`.                                                                                                                                                                                                                                                                                        |
    | `hid_open_path(const char*) -> hid_device*`                   | `hid::open_path(const std::string&) -> std::expected<hid::device, std::wstring>`                                                         | The unexpected state contains the result of `hid::error()`. The reason to use a free function rather than the `hid::device` constructors is that the constructors can not transmit errors unless they throw (which this library avoids).                                                                                               |
    | `hid_open(uint16_t, uint16_t, const wchar_t*) -> hid_device*` | `hid::open(std::uint16_t, std::uint16_t, const std::optional<std::wstring>& = std::nullopt) -> std::expected<hid::device, std::wstring>` | The unexpected state contains the result of `hid::error()`. The reason to use a free function rather than the `hid::device` constructors is that the constructors can not transmit errors unless they throw (which this library avoids).                                                                                               |
    | N/A                                                           | `hid::open(const hid::device_info&) -> std::expected<hid::device, std::wstring>`                                                         | Convenience for `hid::open_path(device_info.path)` and (if that fails) for `hid::open(device_info.vendor_id, device_info.product_id, device_info.serial_number)`.                                                                                                                                                                      |
  - **Device functions**: Functions accepting a `hid_device*` as their first parameter are members of the `hid::device` class which encapsulates a `hid_device*`.
    - TODO