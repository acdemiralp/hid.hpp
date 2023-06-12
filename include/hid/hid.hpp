#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <expected>
#include <format>
#include <optional>
#include <ostream>
#include <ratio>
#include <span>
#include <string>
#include <vector>

#include <hidapi/hidapi.h>

namespace hid
{
using api_version = hid_api_version;

enum class bus_type
{
  unknown   = HID_API_BUS_UNKNOWN,
  usb       = HID_API_BUS_USB,
  bluetooth = HID_API_BUS_BLUETOOTH,
  i2c       = HID_API_BUS_I2C,
  spi       = HID_API_BUS_SPI
};

struct device_info
{
  std::string   path;
  std::uint16_t vendor_id;
  std::uint16_t product_id;
  std::wstring  serial_number;
  std::uint16_t release_number;
  std::wstring  manufacturer_string;
  std::wstring  product_string;
  std::uint16_t usage_page;
  std::uint16_t usage;
  std::int32_t  interface_number;
  bus_type      bus_type;
};

class device
{
public:
  explicit device  (hid_device* native) noexcept
  : native_(native)
  {
    
  }
  device           (const device&  that) = delete;
  device           (      device&& temp) noexcept
  : native_(temp.native_)
  {
    temp.native_ = nullptr;
  }
  virtual ~device  () noexcept
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
      temp.native_ = nullptr;
    }
    return *this;
  }

  // Accessors.

  [[nodiscard]]
  const hid_device*                                      native             () const noexcept
  {
    return native_;
  }

  [[nodiscard]]
  std::expected<device_info, std::wstring>               device_info        () const
  {
    if (const auto info = hid_get_device_info(native_))
      return hid::device_info
      {
        info->path               ,
        info->vendor_id          ,
        info->product_id         ,
        info->serial_number      ,
        info->release_number     ,
        info->manufacturer_string,
        info->product_string     ,
        info->usage_page         ,
        info->usage              ,
        info->interface_number   ,
        static_cast<bus_type>(info->bus_type)
      };
    return std::unexpected(error());
  }

  [[nodiscard]]
  std::expected<std::wstring, std::wstring>              serial_number      (const std::size_t length = 256) const
  {
    std::wstring result(length, '\0');
    if (hid_get_serial_number_string(native_, result.data(), result.size()) == 0)
      return result;
    return std::unexpected(error());
  }
  [[nodiscard]]
  std::expected<std::wstring, std::wstring>              manufacturer_string(const std::size_t length = 256) const
  {
    std::wstring result(length, '\0');
    if (hid_get_manufacturer_string(native_, result.data(), result.size()) == 0)
      return result;
    return std::unexpected(error());
  }
  [[nodiscard]]
  std::expected<std::wstring, std::wstring>              product_string     (const std::size_t length = 256) const
  {
    std::wstring result(length, '\0');
    if (hid_get_product_string(native_, result.data(), result.size()) == 0)
      return result;
    return std::unexpected(error());
  }

  [[nodiscard]]
  std::expected<std::wstring, std::wstring>              indexed_string     (const std::int32_t index, const std::size_t length = 256) const
  {
    std::wstring result(length, '\0');
    if (hid_get_indexed_string(native_, index, result.data(), result.size()) == 0)
      return result;
    return std::unexpected(error());
  }

  [[nodiscard]]
  std::expected<std::vector<std::uint8_t>, std::wstring> report_descriptor  () const
  {
    std::vector<std::uint8_t> result(HID_API_MAX_REPORT_DESCRIPTOR_SIZE, '\0');
    if (const auto size = hid_get_report_descriptor(native_, result.data(), result.size()); size >= 0)
    {
      result.resize(size);
      return result;
    }
    return std::unexpected(error());
  }

  [[nodiscard]]
  std::expected<std::vector<std::uint8_t>, std::wstring> input_report       (const std::uint8_t report_id, const std::size_t length = 256) const
  {
    std::vector<std::uint8_t> result(length, '\0');
    result[0] = report_id;
    if (const auto size = hid_get_input_report(native_, result.data(), result.size()); size >= 0)
    {
      result.resize(size);
      return result;
    }
    return std::unexpected(error());
  }
  [[nodiscard]]
  std::expected<std::vector<std::uint8_t>, std::wstring> feature_report     (const std::uint8_t report_id, const std::size_t length = 256) const
  {
    std::vector<std::uint8_t> result(length, '\0');
    result[0] = report_id;
    if (const auto size = hid_get_feature_report(native_, result.data(), result.size()); size >= 0)
    {
      result.resize(size);
      return result;
    }
    return std::unexpected(error());
  }

  [[nodiscard]]
  std::expected<std::vector<std::uint8_t>, std::wstring> read               (const std::size_t length = 256) const
  {
    std::vector<std::uint8_t> result(length, '\0');
    if (const auto size = hid_read(native_, result.data(), result.size()); size >= 0)
    {
      result.resize(size);
      return result;
    }
    return std::unexpected(error());
  }
  [[nodiscard]]
  std::expected<std::vector<std::uint8_t>, std::wstring> read               (const std::chrono::duration<std::int32_t, std::milli>& timeout, const std::size_t length = 256) const
  {
    std::vector<std::uint8_t> result(length, '\0');
    if (const auto size = hid_read_timeout(native_, result.data(), result.size(), timeout.count()); size >= 0)
    {
      result.resize(size);
      return result;
    }
    return std::unexpected(error());
  }

  // Mutators.

  std::expected<std::int32_t, std::wstring>              write              (const std::span<std::uint8_t>& data) const
  {
    if (const auto size = hid_write(native_, data.data(), data.size()); size >= 0)
      return size;
    return std::unexpected(error());
  }

  std::expected<std::int32_t, std::wstring>              send_feature_report(const std::span<std::uint8_t>& data) const
  {
    if (const auto size = hid_send_feature_report(native_, data.data(), data.size()); size >= 0)
      return size;
    return std::unexpected(error());
  }

  std::expected<void, std::wstring>                      set_nonblocking    (const bool nonblocking) const
  {
    if (hid_set_nonblocking(native_, nonblocking) == 0)
      return {};
    return std::unexpected(error());
  }

protected:
  [[nodiscard]]
  std::wstring                                           error              () const
  {
    return hid_error(native_);
  }

  hid_device* native_;
};

inline std::wstring                        error      ()
{
  return hid_error(nullptr);
}

inline std::expected<void, std::wstring>   init       ()
{
  if (hid_init() == 0)
    return {};
  return std::unexpected(error());
}
inline std::expected<void, std::wstring>   exit       ()
{
  if (hid_exit() == 0)
    return {};
  return std::unexpected(error());
}

inline std::vector<device_info>            enumerate  (const std::uint16_t vendor_id = 0, const std::uint16_t product_id = 0)
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
        iterator->interface_number   ,
        static_cast<bus_type>(iterator->bus_type));
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

inline std::expected<device, std::wstring> open       (const std::uint16_t vendor_id    , const std::uint16_t product_id    , const std::optional<std::wstring>& serial_number = std::nullopt)
{
  if (const auto native = hid_open(vendor_id, product_id, serial_number ? serial_number.value().c_str() : nullptr); native != nullptr)
    return device(native);
  return std::unexpected(error());
}
inline std::expected<device, std::wstring> open       (const std::string& path)
{
  if (const auto native = hid_open_path(path.c_str()); native != nullptr)
    return device(native);
  return std::unexpected(error());
}
inline std::expected<device, std::wstring> open       (const device_info& info)
{
  if (auto result = open(info.path); result.has_value())
    return result;
  return open(info.vendor_id, info.product_id, !info.serial_number.empty() ? info.serial_number : std::optional<std::wstring>(std::nullopt));
}

inline const api_version&                  version    () noexcept
{
  return *hid_version();
}
inline std::string                         version_str()
{
  return hid_version_str();
}

// Stream utilities.
inline std::string                         to_string  (const api_version& value)
{
  return std::to_string(value.major) + "." + std::to_string(value.minor) + "." + std::to_string(value.patch);
}
inline std::string                         to_string  (const bus_type&    value)
{
  static const std::array<std::string, 5> strings
  {
    "unknown",
    "usb",
    "bluetooth",
    "i2c",
    "spi"
  };
  return strings[static_cast<std::size_t>(value)];
}
inline std::wstring                        to_wstring (const device_info& value)
{
  const auto bus_type_str = to_string(value.bus_type);
  return
    std::wstring(L"path: ")                + std::wstring(value.path.begin(), value.path.end())     + L"\n" +
    std::wstring(L"vendor_id: ")           + std::format(L"{:#04x}", value.vendor_id)               + L"\n" +
    std::wstring(L"product_id: ")          + std::format(L"{:#04x}", value.product_id)              + L"\n" +
    std::wstring(L"serial_number: ")       + value.serial_number                                    + L"\n" +
    std::wstring(L"release_number: ")      + std::format(L"{:#04x}", value.release_number)          + L"\n" +
    std::wstring(L"manufacturer_string: ") + value.manufacturer_string                              + L"\n" +
    std::wstring(L"product_string: ")      + value.product_string                                   + L"\n" +
    std::wstring(L"usage_page: ")          + std::format(L"{:#04x}", value.usage_page)              + L"\n" +
    std::wstring(L"usage: ")               + std::format(L"{:#04x}", value.usage)                   + L"\n" +
    std::wstring(L"interface_number: ")    + std::to_wstring(value.interface_number)                + L"\n" +
    std::wstring(L"bus_type: ")            + std::wstring(bus_type_str.begin(), bus_type_str.end()) + L"\n" ;
}
}

template <typename type>
std::basic_ostream<type>& operator<<(std::basic_ostream<type>& stream, const hid::api_version& value)
{
  stream << hid::to_string(value).c_str();
  return stream;
}
template <typename type>
std::basic_ostream<type>& operator<<(std::basic_ostream<type>& stream, const hid::bus_type&    value)
{
  stream << hid::to_string(value).c_str();
  return stream;
}
template <typename type>
std::basic_ostream<type>& operator<<(std::basic_ostream<type>& stream, const hid::device_info& value)
{
  stream << hid::to_wstring(value).c_str();
  return stream;
}