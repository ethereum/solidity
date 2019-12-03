// EVMC: Ethereum Client-VM Connector API.
// Copyright 2019 The EVMC Authors.
// Licensed under the Apache License, Version 2.0.
#pragma once

#include <evmc/evmc.hpp>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace evmc
{
/// The string of bytes.
using bytes = std::basic_string<uint8_t>;

/// Extended value (by dirty flag) for account storage.
struct storage_value
{
    /// The storage value.
    bytes32 value;

    /// True means this value has been modified already by the current transaction.
    bool dirty{false};

    /// Default constructor.
    storage_value() noexcept = default;

    /// Constructor.
    storage_value(const bytes32& _value, bool _dirty = false) noexcept  // NOLINT
      : value{_value}, dirty{_dirty}
    {}
};

/// Mocked account.
struct MockedAccount
{
    /// The account nonce.
    int nonce = 0;

    /// The account code.
    bytes code;

    /// The code hash. Can be a value not related to the actual code.
    bytes32 codehash;

    /// The account balance.
    uint256be balance;

    /// The account storage map.
    std::unordered_map<bytes32, storage_value> storage;

    /// Helper method for setting balance by numeric type.
    void set_balance(uint64_t x) noexcept
    {
        balance = uint256be{};
        for (std::size_t i = 0; i < sizeof(x); ++i)
            balance.bytes[sizeof(balance) - 1 - i] = static_cast<uint8_t>(x >> (8 * i));
    }
};

/// Mocked EVMC Host implementation.
class MockedHost : public Host
{
public:
    /// LOG record.
    struct log_record
    {
        /// The address of the account which created the log.
        address creator;

        /// The data attached to the log.
        bytes data;

        /// The log topics.
        std::vector<bytes32> topics;

        /// Equal operator.
        bool operator==(const log_record& other) const noexcept
        {
            return creator == other.creator && data == other.data && topics == other.topics;
        }
    };

    /// SELFDESTRUCT record.
    struct selfdestuct_record
    {
        /// The address of the account which has self-destructed.
        address selfdestructed;

        /// The address of the beneficiary account.
        address beneficiary;

        /// Equal operator.
        bool operator==(const selfdestuct_record& other) const noexcept
        {
            return selfdestructed == other.selfdestructed && beneficiary == other.beneficiary;
        }
    };

    /// The set of all accounts in the Host, organized by their addresses.
    std::unordered_map<address, MockedAccount> accounts;

    /// The EVMC transaction context to be returned by get_tx_context().
    evmc_tx_context tx_context = {};

    /// The block header hash value to be returned by get_block_hash().
    bytes32 block_hash = {};

    /// The call result to be returned by the call() method.
    evmc_result call_result = {};

    /// The record of all block numbers for which get_block_hash() was called.
    mutable std::vector<int64_t> recorded_blockhashes;

    /// The record of all account accesses.
    mutable std::vector<address> recorded_account_accesses;

    /// The maximum number of entries in recorded_account_accesses record.
    /// This is arbitrary value useful in fuzzing when we don't want the record to explode.
    static constexpr auto max_recorded_account_accesses = 200;

    /// The record of all call messages requested in the call() method.
    std::vector<evmc_message> recorded_calls;

    /// The maximum number of entries in recorded_calls record.
    /// This is arbitrary value useful in fuzzing when we don't want the record to explode.
    static constexpr auto max_recorded_calls = 100;

    /// The record of all LOGs passed to the emit_log() method.
    std::vector<log_record> recorded_logs;

    /// The record of all SELFDESTRUCTs from the selfdestruct() method.
    std::vector<selfdestuct_record> recorded_selfdestructs;

protected:
    /// The copy of call inputs for the recorded_calls record.
    std::vector<bytes> m_recorded_calls_inputs;

    /// Record an account access.
    /// @param addr  The address of the accessed account.
    void record_account_access(const address& addr) const
    {
        if (recorded_account_accesses.empty())
            recorded_account_accesses.reserve(max_recorded_account_accesses);

        if (recorded_account_accesses.size() < max_recorded_account_accesses)
            recorded_account_accesses.emplace_back(addr);
    }

    /// Returns true if an account exists (EVMC Host method).
    bool account_exists(const address& addr) const noexcept override
    {
        record_account_access(addr);
        return accounts.count(addr) != 0;
    }

    /// Get the account's storage value at the given key (EVMC Host method).
    bytes32 get_storage(const address& addr, const bytes32& key) const noexcept override
    {
        record_account_access(addr);

        const auto account_iter = accounts.find(addr);
        if (account_iter == accounts.end())
            return {};

        const auto storage_iter = account_iter->second.storage.find(key);
        if (storage_iter != account_iter->second.storage.end())
            return storage_iter->second.value;
        return {};
    }

    /// Set the account's storage value (EVMC Host method).
    evmc_storage_status set_storage(const address& addr,
                                    const bytes32& key,
                                    const bytes32& value) noexcept override
    {
        record_account_access(addr);
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return EVMC_STORAGE_UNCHANGED;

        auto& old = it->second.storage[key];

        // Follow https://eips.ethereum.org/EIPS/eip-1283 specification.
        // WARNING! This is not complete implementation as refund is not handled here.

        if (old.value == value)
            return EVMC_STORAGE_UNCHANGED;

        evmc_storage_status status{};
        if (!old.dirty)
        {
            old.dirty = true;
            if (!old.value)
                status = EVMC_STORAGE_ADDED;
            else if (value)
                status = EVMC_STORAGE_MODIFIED;
            else
                status = EVMC_STORAGE_DELETED;
        }
        else
            status = EVMC_STORAGE_MODIFIED_AGAIN;

        old.value = value;
        return status;
    }

    /// Get the account's balance (EVMC Host method).
    uint256be get_balance(const address& addr) const noexcept override
    {
        record_account_access(addr);
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return {};

        return it->second.balance;
    }

    /// Get the account's code size (EVMC host method).
    size_t get_code_size(const address& addr) const noexcept override
    {
        record_account_access(addr);
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return 0;
        return it->second.code.size();
    }

    /// Get the account's code hash (EVMC host method).
    bytes32 get_code_hash(const address& addr) const noexcept override
    {
        record_account_access(addr);
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return {};
        return it->second.codehash;
    }

    /// Copy the account's code to the given buffer (EVMC host method).
    size_t copy_code(const address& addr,
                     size_t code_offset,
                     uint8_t* buffer_data,
                     size_t buffer_size) const noexcept override
    {
        record_account_access(addr);
        const auto it = accounts.find(addr);
        if (it == accounts.end())
            return 0;

        const auto& code = it->second.code;

        if (code_offset >= code.size())
            return 0;

        const auto n = std::min(buffer_size, code.size() - code_offset);

        if (n > 0)
            std::copy_n(&code[code_offset], n, buffer_data);
        return n;
    }

    /// Selfdestruct the account (EVMC host method).
    void selfdestruct(const address& addr, const address& beneficiary) noexcept override
    {
        record_account_access(addr);
        recorded_selfdestructs.push_back({addr, beneficiary});
    }

    /// Call/create other contract (EVMC host method).
    result call(const evmc_message& msg) noexcept override
    {
        record_account_access(msg.destination);

        if (recorded_calls.empty())
        {
            recorded_calls.reserve(max_recorded_calls);
            m_recorded_calls_inputs.reserve(max_recorded_calls);  // Iterators will not invalidate.
        }

        if (recorded_calls.size() < max_recorded_calls)
        {
            recorded_calls.emplace_back(msg);
            auto& call_msg = recorded_calls.back();
            if (call_msg.input_size > 0)
            {
                m_recorded_calls_inputs.emplace_back(call_msg.input_data, call_msg.input_size);
                const auto& input_copy = m_recorded_calls_inputs.back();
                call_msg.input_data = input_copy.data();
            }
        }
        return result{call_result};
    }

    /// Get transaction context (EVMC host method).
    evmc_tx_context get_tx_context() const noexcept override { return tx_context; }

    /// Get the block header hash (EVMC host method).
    bytes32 get_block_hash(int64_t block_number) const noexcept override
    {
        recorded_blockhashes.emplace_back(block_number);
        return block_hash;
    }

    /// Emit LOG (EVMC host method).
    void emit_log(const address& addr,
                  const uint8_t* data,
                  size_t data_size,
                  const bytes32 topics[],
                  size_t topics_count) noexcept override
    {
        recorded_logs.push_back({addr, {data, data_size}, {topics, topics + topics_count}});
    }
};
}  // namespace evmc
