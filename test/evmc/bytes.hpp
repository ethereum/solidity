// EVMC: Ethereum Client-VM Connector API.
// Copyright 2024 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <algorithm>
#include <cstring>
#include <string>
#include <string_view>

namespace evmc
{
/// The char traits for byte-like types.
///
/// See: https://en.cppreference.com/w/cpp/string/char_traits.
template <typename T>
struct byte_traits : std::char_traits<char>
{
    static_assert(sizeof(T) == 1, "type must be a byte");

    using char_type = T;  ///< The byte type.

    /// Assigns c2 to c1.
    static constexpr void assign(char_type& c1, const char_type& c2) { c1 = c2; }

    /// Assigns value to each byte in [ptr, ptr+count).
    static constexpr char_type* assign(char_type* ptr, std::size_t count, char_type value)
    {
        std::fill_n(ptr, count, value);
        return ptr;
    }

    /// Returns true if bytes are equal.
    static constexpr bool eq(char_type a, char_type b) { return a == b; }

    /// Returns true if byte a is less than byte b.
    static constexpr bool lt(char_type a, char_type b) { return a < b; }

    /// Copies count bytes from src to dest. Performs correctly even if ranges overlap.
    static constexpr char_type* move(char_type* dest, const char_type* src, std::size_t count)
    {
        if (dest < src)
            std::copy_n(src, count, dest);
        else if (src < dest)
            std::copy_backward(src, src + count, dest + count);
        return dest;
    }

    /// Copies count bytes from src to dest. The ranges must not overlap.
    static constexpr char_type* copy(char_type* dest, const char_type* src, std::size_t count)
    {
        std::copy_n(src, count, dest);
        return dest;
    }

    /// Compares lexicographically the bytes in two ranges of equal length.
    static constexpr int compare(const char_type* a, const char_type* b, std::size_t count)
    {
        for (; count != 0; --count, ++a, ++b)
        {
            if (lt(*a, *b))
                return -1;
            if (lt(*b, *a))
                return 1;
        }
        return 0;
    }

    /// Returns the length of a null-terminated byte string.
    // TODO: Not constexpr
    static std::size_t length(const char_type* s)
    {
        return std::strlen(reinterpret_cast<const char*>(s));
    }

    /// Finds the value in the range of bytes and returns the pointer to the first occurrence
    /// or nullptr if not found.
    static constexpr const char_type* find(const char_type* s,
                                           std::size_t count,
                                           const char_type& value)
    {
        const auto end = s + count;
        const auto p = std::find(s, end, value);
        return p != end ? p : nullptr;
    }
};

/// String of unsigned chars representing bytes.
using bytes = std::basic_string<unsigned char, byte_traits<unsigned char>>;

/// String view of unsigned chars representing bytes.
using bytes_view = std::basic_string_view<unsigned char, byte_traits<unsigned char>>;
}  // namespace evmc
