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

#include <evmc/evmc.hpp>

#include <cstring>
#include <functional>

using evmc::is_zero;

/// The comparator for std::map<evmc_address, ...>.
EVMC_DEPRECATED
inline bool operator<(const evmc_address& a, const evmc_address& b)
{
    return std::memcmp(a.bytes, b.bytes, sizeof(a.bytes)) < 0;
}

/// The comparator for std::map<evmc_bytes32, ...>.
EVMC_DEPRECATED
inline bool operator<(const evmc_bytes32& a, const evmc_bytes32& b)
{
    return std::memcmp(a.bytes, b.bytes, sizeof(a.bytes)) < 0;
}

/// The comparator for equality.
EVMC_DEPRECATED
inline bool operator==(const evmc_address& a, const evmc_address& b)
{
    return std::memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0;
}

/// The comparator for equality.
EVMC_DEPRECATED
inline bool operator==(const evmc_bytes32& a, const evmc_bytes32& b)
{
    return std::memcmp(a.bytes, b.bytes, sizeof(a.bytes)) == 0;
}

/// Parameters for the fnv1a hash function, specialized by the hash result size (size_t).
///
/// The values for the matching size are taken from
/// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters.
///
/// @tparam size  The size of the hash result (size_t).
template <size_t size>
struct fnv1_params
{
};

/// Parameters for the fnv1a hash function, specialized for the hash result of 4 bytes.
template <>
struct fnv1_params<4>
{
    static constexpr auto prime = 0x1000193;          ///< The FNV prime.
    static constexpr auto offset_basis = 0x811c9dc5;  ///< The FNV offset basis.
};

/// Parameters for the fnv1a hash function, specialized for the hash result of 8 bytes.
template <>
struct fnv1_params<8>
{
    static constexpr auto prime = 0x100000001b3;              ///< The FNV prime.
    static constexpr auto offset_basis = 0xcbf29ce484222325;  ///< The FNV offset basis.
};

/// FNV1a hash function.
inline size_t fnv1a(const uint8_t* ptr, size_t len) noexcept
{
    using params = fnv1_params<sizeof(size_t)>;

    auto ret = size_t{params::offset_basis};
    for (size_t i = 0; i < len; i++)
    {
        ret ^= ptr[i];
        ret *= params::prime;
    }
    return ret;
}

namespace std
{
/// Hash operator template specialization for evmc_address needed for unordered containers.
template <>
struct EVMC_DEPRECATED hash<evmc_address>
{
    /// Hash operator using FNV1a.
    size_t operator()(const evmc_address& s) const noexcept
    {
        return fnv1a(s.bytes, sizeof(s.bytes));
    }
};

/// Hash operator template needed for std::unordered_set and others using hashes.
template <>
struct EVMC_DEPRECATED hash<evmc_bytes32>
{
    /// Hash operator using FNV1a.
    size_t operator()(const evmc_bytes32& s) const noexcept
    {
        return fnv1a(s.bytes, sizeof(s.bytes));
    }
};
}  // namespace std

/** @} */
