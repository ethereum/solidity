/* EVMC: Ethereum Client-VM Connector API.
 * Copyright 2018-2019 The EVMC Authors.
 * Licensed under the Apache License, Version 2.0.
 */

/**
 * @file
 * A collection of helpers (overloaded operators) for using EVMC types effectively in C++.
 *
 * @addtogroup helpers
 * @{
 */
#pragma once

#include <evmc/evmc.h>

#include <cstring>
#include <functional>

/// The comparator for std::map<evmc_address, ...>.
inline bool operator<(const evmc_address& a, const evmc_address& b)
{
    return std::memcmp(a.bytes, b.bytes, sizeof(a.bytes)) < 0;
}

/// The comparator for std::map<evmc_bytes32, ...>.
inline bool operator<(const evmc_bytes32& a, const evmc_bytes32& b)
{
    return std::memcmp(a.bytes, b.bytes, sizeof(a.bytes)) < 0;
}

/// The comparator for equality.
inline bool operator==(const evmc_address& a, const evmc_address& b)
{
    return std::memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0;
}

/// The comparator for equality.
inline bool operator==(const evmc_bytes32& a, const evmc_bytes32& b)
{
    return std::memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0;
}

/// Check if the address is zero (all bytes are zeros).
inline bool is_zero(const evmc_address& address) noexcept
{
    return address == evmc_address{};
}

/// Check if the hash is zero (all bytes are zeros).
inline bool is_zero(const evmc_bytes32& x) noexcept
{
    return x == evmc_bytes32{};
}

/// FNV1a hash function with 64-bit result.
inline uint64_t fnv1a_64(const uint8_t* ptr, size_t len)
{
    constexpr uint64_t prime = 1099511628211ULL;
    constexpr uint64_t offset_basis = 14695981039346656037ULL;

    uint64_t ret = offset_basis;
    for (size_t i = 0; i < len; i++)
    {
        ret ^= ptr[i];
        ret *= prime;
    }
    return ret;
}

namespace std
{
/// Hash operator template specialization for evmc_address needed for unordered containers.
template <>
struct hash<evmc_address>
{
    /// Hash operator using FNV1a.
    std::enable_if<sizeof(size_t) == 8, std::size_t>::type operator()(const evmc_address& s) const
        noexcept
    {
        return fnv1a_64(s.bytes, sizeof(s.bytes));
    }
};

/// Hash operator template needed for std::unordered_set and others using hashes.
template <>
struct hash<evmc_bytes32>
{
    /// Hash operator using FNV1a.
    std::enable_if<sizeof(size_t) == 8, std::size_t>::type operator()(const evmc_bytes32& s) const
        noexcept
    {
        return fnv1a_64(s.bytes, sizeof(s.bytes));
    }
};
}  // namespace std

/** @} */
