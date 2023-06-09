#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <hidapi/hidapi.h>

namespace hid
{
using api_version = hid_api_version;

enum class bus_type
{
  unknown   = HID_API_BUS_UNKNOWN  ,
  usb       = HID_API_BUS_USB      ,
  bluetooth = HID_API_BUS_BLUETOOTH,
  i2c       = HID_API_BUS_I2C      ,
  spi       = HID_API_BUS_SPI
};

struct device_info
{
  std::string   path               ;
  std::uint16_t vendor_id          ;
  std::uint16_t product_id         ;
  std::wstring  serial_number      ;
  std::uint16_t release_number     ;
  std::wstring  manufacturer_string;
  std::wstring  product_string     ;
  std::uint16_t usage_page         ;
  std::uint16_t usage              ;
  std::int32_t  interface_number   ;
  bus_type      bus_type           ;
};

class device
{
public:
  explicit device  (hid_device* native)
  : native_(native)
  {
    
  }
  device           (const device&  that) = delete;
  device           (      device&& temp) noexcept
  : native_(temp.native_)
  {
    temp.native_ = nullptr;
  }
  virtual ~device  ()
  {
    if (native_)
      hid_close(native_);
  }
  device& operator=(const device&  that) = delete;
  device& operator=(      device&& temp) noexcept
  {
    if (this != &temp)
    {
      native_      = temp.native_;
      temp.native_ = nullptr     ;
    }
    return *this;
  }

  void              write (const std::span<std::uint8_t>& data) const
  {
    hid_write(native_, data.data(), data.size());
  }
  void              read  (      std::span<std::uint8_t>& data) const
  {
    hid_read (native_, data.data(), data.size());
  }

  [[nodiscard]]
  std::wstring      error () const noexcept
  {
    return hid_error(native_);
  }

  [[nodiscard]]
  const hid_device* native() const noexcept
  {
    return native_;
  }

protected:
  hid_device* native_;
};

inline std::wstring                        error      () noexcept
{
  return hid_error(nullptr);
}
inline const api_version*                  version    () noexcept
{
  return hid_version();
}
inline std::string                         version_str() noexcept
{
  return hid_version_str();
}

inline std::expected<void  , std::wstring> init       () noexcept
{
  if (hid_init() == 0)
    return {};
  return std::unexpected(error());
}
inline std::expected<void  , std::wstring> exit       () noexcept
{
  if (hid_exit() == 0)
    return {};
  return std::unexpected(error());
}

inline std::vector  <device_info>          enumerate  (const std::uint16_t vendor_id = 0, const std::uint16_t product_id = 0)
{
  std::vector<device_info> result;

  if (const auto device_info = hid_enumerate(vendor_id, product_id))
  {
    auto iterator = device_info;
    while (iterator != nullptr)
    {
      result.emplace_back(
        iterator->path               ,
        iterator->vendor_id          ,
        iterator->product_id         ,
        iterator->serial_number      ,
        iterator->release_number     ,
        iterator->manufacturer_string,
        iterator->product_string     ,
        iterator->usage_page         ,
        iterator->usage              ,
        iterator->interface_number   );
      iterator = iterator->next;
    }

    hid_free_enumeration(device_info);
  }

  // "This function returns NULL in the case of failure or if no HID devices present in the system."
  // A nullptr is not necessarily due to an error, but simply due to no matching devices.
  // Hence just return an empty container instead of an error state.
  // The user may call error() manually to check the reason.

  return result;
}

inline std::expected<device, std::wstring> open       (const std::string&  path) noexcept
{
  if (const auto native = hid_open_path(path.c_str()); native != nullptr)
    return device(native);
  return std::unexpected(error());
}
inline std::expected<device, std::wstring> open       (const std::uint16_t vendor_id    , const std::uint16_t product_id    , const std::optional<std::wstring>& serial_number = std::nullopt) noexcept
{
  if (const auto native = hid_open(vendor_id, product_id, serial_number ? serial_number.value().c_str() : nullptr); native != nullptr)
    return device(native);
  return std::unexpected(error());
}
inline std::expected<device, std::wstring> open       (const device_info&  info) noexcept
{
  if (auto result = open(info.path); result.has_value())
    return result;
  return open(info.vendor_id, info.product_id, !info.serial_number.empty() ? info.serial_number : std::optional<std::wstring>(std::nullopt));
}
}