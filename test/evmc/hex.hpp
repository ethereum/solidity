// EVMC: Ethereum Client-VM Connector API.
// Copyright 2021 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <evmc/filter_iterator.hpp>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace evmc
{
/// String of uint8_t chars.
using bytes = std::basic_string<uint8_t>;

/// String view of uint8_t chars.
using bytes_view = std::basic_string_view<uint8_t>;


/// Encode a byte to a hex string.
inline std::string hex(uint8_t b) noexcept
{
    static constexpr auto hex_digits = "0123456789abcdef";
    return {hex_digits[b >> 4], hex_digits[b & 0xf]};
}

/// Encodes bytes as hex string.
inline std::string hex(bytes_view bs)
{
    std::string str;
    str.reserve(bs.size() * 2);
    for (const auto b : bs)
        str += hex(b);
    return str;
}

namespace internal
{
/// Extracts the nibble value out of a hex digit.
/// Returns -1 in case of invalid hex digit.
inline constexpr int from_hex_digit(char h) noexcept
{
    if (h >= '0' && h <= '9')
        return h - '0';
    else if (h >= 'a' && h <= 'f')
        return h - 'a' + 10;
    else if (h >= 'A' && h <= 'F')
        return h - 'A' + 10;
    else
        return -1;
}
}  // namespace internal

/// Decodes hex-encoded sequence of characters.
///
/// It is guaranteed that the output will not be longer than half of the input length.
///
/// @param begin  The input begin iterator. It only must satisfy input iterator concept.
/// @param end    The input end iterator. It only must satisfy input iterator concept.
/// @param out    The output iterator. It must satisfy output iterator concept.
/// @return       True if successful, false if input is invalid hex.
template <typename InputIt, typename OutputIt>
inline constexpr bool from_hex(InputIt begin, InputIt end, OutputIt out) noexcept
{
    int hi_nibble = -1;  // Init with invalid value, should never be used.
    size_t i = 0;
    for (auto it = begin; it != end; ++it, ++i)
    {
        const auto h = *it;
        const int v = evmc::internal::from_hex_digit(h);
        if (v < 0)
        {
            if (i == 1 && hi_nibble == 0 && h == 'x')  // 0x prefix
                continue;
            return false;
        }

        if (i % 2 == 0)
            hi_nibble = v << 4;
        else
            *out++ = static_cast<uint8_t>(hi_nibble | v);
    }

    return i % 2 == 0;
}

/// Validates hex encoded string.
///
/// @return  True if the input is valid hex.
inline bool validate_hex(std::string_view hex) noexcept
{
    struct noop_output_iterator
    {
        uint8_t sink = {};
        uint8_t& operator*() noexcept { return sink; }
        noop_output_iterator operator++(int) noexcept { return *this; }  // NOLINT(cert-dcl21-cpp)
    };

    return from_hex(hex.begin(), hex.end(), noop_output_iterator{});
}

/// Decodes hex encoded string to bytes.
///
/// In case the input is invalid the returned value is std::nullopt.
/// This can happen if a non-hex digit or odd number of digits is encountered.
inline std::optional<bytes> from_hex(std::string_view hex)
{
    bytes bs;
    bs.reserve(hex.size() / 2);
    if (!from_hex(hex.begin(), hex.end(), std::back_inserter(bs)))
        return {};
    return bs;
}

/// Decodes hex-encoded string into custom type T with .bytes array of uint8_t.
///
/// When the input is smaller than the result type, the result is padded with zeros on the left
/// (the result bytes of lowest indices are filled with zeros).
/// TODO: Support optional left alignment.
template <typename T>
constexpr std::optional<T> from_hex(std::string_view s) noexcept
{
    // Omit the optional 0x prefix.
    if (s.size() >= 2 && s[0] == '0' && s[1] == 'x')
        s.remove_prefix(2);

    T r{};  // The T must have .bytes array. This may be lifted if std::bit_cast is available.
    constexpr auto num_out_bytes = std::size(r.bytes);
    const auto num_in_bytes = s.length() / 2;
    if (num_in_bytes > num_out_bytes)
        return {};
    if (!from_hex(s.begin(), s.end(), &r.bytes[num_out_bytes - num_in_bytes]))
        return {};
    return r;
}

/// Decodes hex encoded string to bytes. The whitespace in the input is ignored.
///
/// In case the input is invalid the returned value is std::nullopt.
/// This can happen if a non-hex digit or odd number of digits is encountered.
/// The whitespace (as defined by std::isspace) in the input is ignored.
template <typename InputIterator>
std::optional<bytes> from_spaced_hex(InputIterator begin, InputIterator end) noexcept
{
    bytes bs;
    if (!from_hex(skip_space_iterator{begin, end}, skip_space_iterator{end, end},
                  std::back_inserter(bs)))
        return {};
    return bs;
}

/// @copydoc from_spaced_hex
inline std::optional<bytes> from_spaced_hex(std::string_view hex) noexcept
{
    return from_spaced_hex(hex.begin(), hex.end());
}
}  // namespace evmc
