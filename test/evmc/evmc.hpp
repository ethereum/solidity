/* EVMC: Ethereum Client-VM Connector API.
 * Copyright 2018-2019 The EVMC Authors.
 * Licensed under the Apache License, Version 2.0.
 */
#pragma once

#include <evmc/evmc.h>
#include <evmc/helpers.h>

#include <functional>
#include <initializer_list>
#include <utility>

/// EVMC C++ API - wrappers and bindings for C++
/// @ingroup cpp
namespace evmc
{
/// The big-endian 160-bit hash suitable for keeping an Ethereum address.
///
/// This type wraps C ::evmc_address to make sure objects of this type are always initialized.
struct address : evmc_address
{
    /// Default and converting constructor.
    ///
    /// Initializes bytes to zeros if not other @p init value provided.
    constexpr address(evmc_address init = {}) noexcept : evmc_address{init} {}

    /// Explicit operator converting to bool.
    constexpr inline explicit operator bool() const noexcept;
};

/// The fixed size array of 32 bytes for storing 256-bit EVM values.
///
/// This type wraps C ::evmc_bytes32 to make sure objects of this type are always initialized.
struct bytes32 : evmc_bytes32
{
    /// Default and converting constructor.
    ///
    /// Initializes bytes to zeros if not other @p init value provided.
    constexpr bytes32(evmc_bytes32 init = {}) noexcept : evmc_bytes32{init} {}

    /// Explicit operator converting to bool.
    constexpr inline explicit operator bool() const noexcept;
};

/// The alias for evmc::bytes32 to represent a big-endian 256-bit integer.
using uint256be = bytes32;


/// Loads 64 bits / 8 bytes of data from the given @p bytes array in big-endian order.
constexpr inline uint64_t load64be(const uint8_t* bytes) noexcept
{
    // TODO: Report bug in clang incorrectly optimizing this with AVX2 enabled.
    return (uint64_t{bytes[0]} << 56) | (uint64_t{bytes[1]} << 48) | (uint64_t{bytes[2]} << 40) |
           (uint64_t{bytes[3]} << 32) | (uint64_t{bytes[4]} << 24) | (uint64_t{bytes[5]} << 16) |
           (uint64_t{bytes[6]} << 8) | uint64_t{bytes[7]};
}

/// Loads 32 bits / 4 bytes of data from the given @p bytes array in big-endian order.
constexpr inline uint32_t load32be(const uint8_t* bytes) noexcept
{
    return (uint32_t{bytes[0]} << 24) | (uint32_t{bytes[1]} << 16) | (uint32_t{bytes[2]} << 8) |
           uint32_t{bytes[3]};
}

namespace fnv
{
constexpr auto prime = 0x100000001b3;              ///< The 64-bit FNV prime number.
constexpr auto offset_basis = 0xcbf29ce484222325;  ///< The 64-bit FNV offset basis.

/// The hashing transformation for 64-bit inputs based on the FNV-1a formula.
constexpr inline uint64_t fnv1a_by64(uint64_t h, uint64_t x) noexcept
{
    return (h ^ x) * prime;
}
}  // namespace fnv


/// The "equal" comparison operator for the evmc::address type.
constexpr bool operator==(const address& a, const address& b) noexcept
{
    // TODO: Report bug in clang keeping unnecessary bswap.
    return load64be(&a.bytes[0]) == load64be(&b.bytes[0]) &&
           load64be(&a.bytes[8]) == load64be(&b.bytes[8]) &&
           load32be(&a.bytes[16]) == load32be(&b.bytes[16]);
}

/// The "not equal" comparison operator for the evmc::address type.
constexpr bool operator!=(const address& a, const address& b) noexcept
{
    return !(a == b);
}

/// The "less" comparison operator for the evmc::address type.
constexpr bool operator<(const address& a, const address& b) noexcept
{
    return load64be(&a.bytes[0]) < load64be(&b.bytes[0]) ||
           (load64be(&a.bytes[0]) == load64be(&b.bytes[0]) &&
            load64be(&a.bytes[8]) < load64be(&b.bytes[8])) ||
           (load64be(&a.bytes[8]) == load64be(&b.bytes[8]) &&
            load32be(&a.bytes[16]) < load32be(&b.bytes[16]));
}

/// The "equal" comparison operator for the evmc::bytes32 type.
constexpr bool operator==(const bytes32& a, const bytes32& b) noexcept
{
    return load64be(&a.bytes[0]) == load64be(&b.bytes[0]) &&
           load64be(&a.bytes[8]) == load64be(&b.bytes[8]) &&
           load64be(&a.bytes[16]) == load64be(&b.bytes[16]) &&
           load64be(&a.bytes[24]) == load64be(&b.bytes[24]);
}

/// The "not equal" comparison operator for the evmc::bytes32 type.
constexpr bool operator!=(const bytes32& a, const bytes32& b) noexcept
{
    return !(a == b);
}

/// The "less" comparison operator for the evmc::bytes32 type.
constexpr bool operator<(const bytes32& a, const bytes32& b) noexcept
{
    return load64be(&a.bytes[0]) < load64be(&b.bytes[0]) ||
           (load64be(&a.bytes[0]) == load64be(&b.bytes[0]) &&
            load64be(&a.bytes[8]) < load64be(&b.bytes[8])) ||
           (load64be(&a.bytes[8]) == load64be(&b.bytes[8]) &&
            load64be(&a.bytes[16]) < load64be(&b.bytes[16])) ||
           (load64be(&a.bytes[16]) == load64be(&b.bytes[16]) &&
            load64be(&a.bytes[24]) < load64be(&b.bytes[24]));
}

/// Checks if the given address is the zero address.
constexpr inline bool is_zero(const address& a) noexcept
{
    return a == address{};
}

constexpr address::operator bool() const noexcept
{
    return !is_zero(*this);
}

/// Checks if the given bytes32 object has all zero bytes.
constexpr inline bool is_zero(const bytes32& a) noexcept
{
    return a == bytes32{};
}

constexpr bytes32::operator bool() const noexcept
{
    return !is_zero(*this);
}

namespace literals
{
namespace internal
{
template <typename T, T... Ints>
struct integer_sequence
{
};

template <uint8_t... Bytes>
using byte_sequence = integer_sequence<uint8_t, Bytes...>;

template <char... Chars>
using char_sequence = integer_sequence<char, Chars...>;


template <typename, typename>
struct concatenate;

template <uint8_t... Bytes1, uint8_t... Bytes2>
struct concatenate<byte_sequence<Bytes1...>, byte_sequence<Bytes2...>>
{
    using type = byte_sequence<Bytes1..., Bytes2...>;
};

template <uint8_t D>
constexpr uint8_t parse_hex_digit() noexcept
{
    static_assert((D >= '0' && D <= '9') || (D >= 'a' && D <= 'f') || (D >= 'A' && D <= 'F'),
                  "literal must be hexadecimal integer");
    return static_cast<uint8_t>(
        (D >= '0' && D <= '9') ? D - '0' : (D >= 'a' && D <= 'f') ? D - 'a' + 10 : D - 'A' + 10);
}


template <typename>
struct parse_digits;

template <uint8_t Digit1, uint8_t Digit2>
struct parse_digits<byte_sequence<Digit1, Digit2>>
{
    using type = byte_sequence<static_cast<uint8_t>(parse_hex_digit<Digit1>() << 4) |
                               parse_hex_digit<Digit2>()>;
};

template <uint8_t Digit1, uint8_t Digit2, uint8_t... Rest>
struct parse_digits<byte_sequence<Digit1, Digit2, Rest...>>
{
    using type = typename concatenate<typename parse_digits<byte_sequence<Digit1, Digit2>>::type,
                                      typename parse_digits<byte_sequence<Rest...>>::type>::type;
};


template <typename, typename>
struct parse_literal;

template <typename T, char Prefix1, char Prefix2, char... Literal>
struct parse_literal<T, char_sequence<Prefix1, Prefix2, Literal...>>
{
    static_assert(Prefix1 == '0' && Prefix2 == 'x', "literal must be in hexadecimal notation");
    static_assert(sizeof...(Literal) == sizeof(T) * 2, "literal must match the result type size");

    template <uint8_t... Bytes>
    static constexpr T create_from(byte_sequence<Bytes...>) noexcept
    {
        return T{{{Bytes...}}};
    }

    static constexpr T get() noexcept
    {
        return create_from(typename parse_digits<byte_sequence<Literal...>>::type{});
    }
};

template <typename T, char Digit>
struct parse_literal<T, char_sequence<Digit>>
{
    static_assert(Digit == '0', "only 0 is allowed as a single digit literal");
    static constexpr T get() noexcept { return {}; }
};

template <typename T, char... Literal>
constexpr T parse() noexcept
{
    return parse_literal<T, char_sequence<Literal...>>::get();
}
}  // namespace internal

/// Literal for evmc::address.
template <char... Literal>
constexpr address operator"" _address() noexcept
{
    return internal::parse<address, Literal...>();
}

/// Literal for evmc::bytes32.
template <char... Literal>
constexpr bytes32 operator"" _bytes32() noexcept
{
    return internal::parse<bytes32, Literal...>();
}
}  // namespace literals

using namespace literals;


/// Alias for evmc_make_result().
constexpr auto make_result = evmc_make_result;

/// @copydoc evmc_result
///
/// This is a RAII wrapper for evmc_result and objects of this type
/// automatically release attached resources.
class result : private evmc_result
{
public:
    using evmc_result::create_address;
    using evmc_result::gas_left;
    using evmc_result::output_data;
    using evmc_result::output_size;
    using evmc_result::status_code;

    /// Creates the result from the provided arguments.
    ///
    /// The provided output is copied to memory allocated with malloc()
    /// and the evmc_result::release function is set to one invoking free().
    ///
    /// @param _status_code  The status code.
    /// @param _gas_left     The amount of gas left.
    /// @param _output_data  The pointer to the output.
    /// @param _output_size  The output size.
    result(evmc_status_code _status_code,
           int64_t _gas_left,
           const uint8_t* _output_data,
           size_t _output_size) noexcept
      : evmc_result{make_result(_status_code, _gas_left, _output_data, _output_size)}
    {}

    /// Converting constructor from raw evmc_result.
    explicit result(evmc_result const& res) noexcept : evmc_result{res} {}

    /// Destructor responsible for automatically releasing attached resources.
    ~result() noexcept
    {
        if (release)
            release(this);
    }

    /// Move constructor.
    result(result&& other) noexcept : evmc_result{other}
    {
        other.release = nullptr;  // Disable releasing of the rvalue object.
    }

    /// Move assignment operator.
    ///
    /// The self-assigment MUST never happen.
    ///
    /// @param other The other result object.
    /// @return      The reference to the left-hand side object.
    result& operator=(result&& other) noexcept
    {
        this->~result();                           // Release this object.
        static_cast<evmc_result&>(*this) = other;  // Copy data.
        other.release = nullptr;                   // Disable releasing of the rvalue object.
        return *this;
    }

    /// Releases the ownership and returns the raw copy of evmc_result.
    ///
    /// This method drops the ownership of the result
    /// (result's resources are not going to be released when this object is destructed).
    /// It is the caller's responsibility having the returned copy of the result to release it.
    /// This object MUST NOT be used after this method is invoked.
    ///
    /// @return  The copy of this object converted to raw evmc_result.
    evmc_result release_raw() noexcept
    {
        const auto out = evmc_result{*this};  // Copy data.
        this->release = nullptr;              // Disable releasing of this object.
        return out;
    }
};


/// The EVMC Host interface
class HostInterface
{
public:
    virtual ~HostInterface() noexcept = default;

    /// @copydoc evmc_host_interface::account_exists
    virtual bool account_exists(const address& addr) noexcept = 0;

    /// @copydoc evmc_host_interface::get_storage
    virtual bytes32 get_storage(const address& addr, const bytes32& key) noexcept = 0;

    /// @copydoc evmc_host_interface::set_storage
    virtual evmc_storage_status set_storage(const address& addr,
                                            const bytes32& key,
                                            const bytes32& value) noexcept = 0;

    /// @copydoc evmc_host_interface::get_balance
    virtual uint256be get_balance(const address& addr) noexcept = 0;

    /// @copydoc evmc_host_interface::get_code_size
    virtual size_t get_code_size(const address& addr) noexcept = 0;

    /// @copydoc evmc_host_interface::get_code_hash
    virtual bytes32 get_code_hash(const address& addr) noexcept = 0;

    /// @copydoc evmc_host_interface::copy_code
    virtual size_t copy_code(const address& addr,
                             size_t code_offset,
                             uint8_t* buffer_data,
                             size_t buffer_size) noexcept = 0;

    /// @copydoc evmc_host_interface::selfdestruct
    virtual void selfdestruct(const address& addr, const address& beneficiary) noexcept = 0;

    /// @copydoc evmc_host_interface::call
    virtual result call(const evmc_message& msg) noexcept = 0;

    /// @copydoc evmc_host_interface::get_tx_context
    virtual evmc_tx_context get_tx_context() noexcept = 0;

    /// @copydoc evmc_host_interface::get_block_hash
    virtual bytes32 get_block_hash(int64_t block_number) noexcept = 0;

    /// @copydoc evmc_host_interface::emit_log
    virtual void emit_log(const address& addr,
                          const uint8_t* data,
                          size_t data_size,
                          const bytes32 topics[],
                          size_t num_topics) noexcept = 0;
};


/// Wrapper around EVMC host context / host interface.
///
/// To be used by VM implementations as better alternative to using ::evmc_host_context directly.
class HostContext : public HostInterface
{
    const evmc_host_interface* host = nullptr;
    evmc_host_context* context = nullptr;
    evmc_tx_context tx_context = {};

public:
    /// Default constructor for null Host context.
    HostContext() = default;

    /// Constructor from the EVMC Host primitives.
    /// @param interface  The reference to the Host interface.
    /// @param ctx        The pointer to the Host context object. This parameter MAY be null.
    HostContext(const evmc_host_interface& interface, evmc_host_context* ctx) noexcept
      : host{&interface}, context{ctx}
    {}

    bool account_exists(const address& address) noexcept final
    {
        return host->account_exists(context, &address);
    }

    bytes32 get_storage(const address& address, const bytes32& key) noexcept final
    {
        return host->get_storage(context, &address, &key);
    }

    evmc_storage_status set_storage(const address& address,
                                    const bytes32& key,
                                    const bytes32& value) noexcept final
    {
        return host->set_storage(context, &address, &key, &value);
    }

    uint256be get_balance(const address& address) noexcept final
    {
        return host->get_balance(context, &address);
    }

    size_t get_code_size(const address& address) noexcept final
    {
        return host->get_code_size(context, &address);
    }

    bytes32 get_code_hash(const address& address) noexcept final
    {
        return host->get_code_hash(context, &address);
    }

    size_t copy_code(const address& address,
                     size_t code_offset,
                     uint8_t* buffer_data,
                     size_t buffer_size) noexcept final
    {
        return host->copy_code(context, &address, code_offset, buffer_data, buffer_size);
    }

    void selfdestruct(const address& addr, const address& beneficiary) noexcept final
    {
        host->selfdestruct(context, &addr, &beneficiary);
    }

    result call(const evmc_message& message) noexcept final
    {
        return result{host->call(context, &message)};
    }

    /// @copydoc HostInterface::get_tx_context()
    ///
    /// The implementation caches the received transaction context
    /// by assuming that the block timestamp should never be zero.
    ///
    /// @return The cached transaction context.
    evmc_tx_context get_tx_context() noexcept final
    {
        if (tx_context.block_timestamp == 0)
            tx_context = host->get_tx_context(context);
        return tx_context;
    }

    bytes32 get_block_hash(int64_t number) noexcept final
    {
        return host->get_block_hash(context, number);
    }

    void emit_log(const address& addr,
                  const uint8_t* data,
                  size_t data_size,
                  const bytes32 topics[],
                  size_t topics_count) noexcept final
    {
        host->emit_log(context, &addr, data, data_size, topics, topics_count);
    }
};


/// Abstract class to be used by Host implementations.
///
/// When implementing EVMC Host, you can directly inherit from the evmc::Host class.
/// This way your implementation will be simpler by avoiding manual handling
/// of the ::evmc_host_context and the ::evmc_host_interface.
class Host : public HostInterface
{
public:
    /// Provides access to the global host interface.
    /// @returns  Reference to the host interface object.
    static const evmc_host_interface& get_interface() noexcept;

    /// Converts the Host object to the opaque host context pointer.
    /// @returns  Pointer to evmc_host_context.
    evmc_host_context* to_context() noexcept { return reinterpret_cast<evmc_host_context*>(this); }

    /// Converts the opaque host context pointer back to the original Host object.
    /// @tparam DerivedClass  The class derived from the Host class.
    /// @param context        The opaque host context pointer.
    /// @returns              The pointer to DerivedClass.
    template <typename DerivedClass = Host>
    static DerivedClass* from_context(evmc_host_context* context) noexcept
    {
        // Get pointer of the Host base class.
        auto* h = reinterpret_cast<Host*>(context);

        // Additional downcast, only possible if DerivedClass inherits from Host.
        return static_cast<DerivedClass*>(h);
    }
};


/// @copybrief evmc_vm
///
/// This is a RAII wrapper for evmc_vm, and object of this type
/// automatically destroys the VM instance.
class VM
{
public:
    VM() noexcept = default;

    /// Converting constructor from evmc_vm.
    explicit VM(evmc_vm* vm) noexcept : m_instance{vm} {}

    /// Destructor responsible for automatically destroying the VM instance.
    ~VM() noexcept
    {
        if (m_instance)
            m_instance->destroy(m_instance);
    }

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;

    /// Move constructor.
    VM(VM&& other) noexcept : m_instance{other.m_instance} { other.m_instance = nullptr; }

    /// Move assignment operator.
    VM& operator=(VM&& other) noexcept
    {
        this->~VM();
        m_instance = other.m_instance;
        other.m_instance = nullptr;
        return *this;
    }

    /// The constructor that captures a VM instance and configures the instance
    /// with the provided list of options.
    inline VM(evmc_vm* vm,
              std::initializer_list<std::pair<const char*, const char*>> options) noexcept;

    /// Checks if contains a valid pointer to the VM instance.
    explicit operator bool() const noexcept { return m_instance != nullptr; }

    /// Checks whenever the VM instance is ABI compatible with the current EVMC API.
    bool is_abi_compatible() const noexcept { return m_instance->abi_version == EVMC_ABI_VERSION; }

    /// @copydoc evmc_vm::name
    char const* name() const noexcept { return m_instance->name; }

    /// @copydoc evmc_vm::version
    char const* version() const noexcept { return m_instance->version; }

    /// @copydoc evmc::vm::get_capabilities
    evmc_capabilities_flagset get_capabilities() const noexcept
    {
        return m_instance->get_capabilities(m_instance);
    }

    /// @copydoc evmc_set_option()
    evmc_set_option_result set_option(const char name[], const char value[]) noexcept
    {
        return evmc_set_option(m_instance, name, value);
    }

    /// @copydoc evmc_execute()
    result execute(const evmc_host_interface& host,
                   evmc_host_context* ctx,
                   evmc_revision rev,
                   const evmc_message& msg,
                   const uint8_t* code,
                   size_t code_size) noexcept
    {
        return result{m_instance->execute(m_instance, &host, ctx, rev, &msg, code, code_size)};
    }

    /// Convenient variant of the VM::execute() that takes reference to evmc::Host class.
    result execute(Host& host,
                   evmc_revision rev,
                   const evmc_message& msg,
                   const uint8_t* code,
                   size_t code_size) noexcept
    {
        return execute(Host::get_interface(), host.to_context(), rev, msg, code, code_size);
    }

    /// Executes code without the Host context.
    ///
    /// The same as
    /// execute(const evmc_host_interface&, evmc_host_context*, evmc_revision,
    ///         const evmc_message&, const uint8_t*, size_t),
    /// but without providing the Host context and interface.
    /// This method is for experimental precompiles support where execution is
    /// guaranteed not to require any Host access.
    result execute(evmc_revision rev,
                   const evmc_message& msg,
                   const uint8_t* code,
                   size_t code_size) noexcept
    {
        return result{
            m_instance->execute(m_instance, nullptr, nullptr, rev, &msg, code, code_size)};
    }

private:
    evmc_vm* m_instance = nullptr;
};

inline VM::VM(evmc_vm* vm,
              std::initializer_list<std::pair<const char*, const char*>> options) noexcept
  : m_instance{vm}
{
    // This constructor is implemented outside of the class definition to workaround a doxygen bug.
    for (const auto& option : options)
        set_option(option.first, option.second);
}


namespace internal
{
inline bool account_exists(evmc_host_context* h, const evmc_address* addr) noexcept
{
    return Host::from_context(h)->account_exists(*addr);
}

inline evmc_bytes32 get_storage(evmc_host_context* h,
                                const evmc_address* addr,
                                const evmc_bytes32* key) noexcept
{
    return Host::from_context(h)->get_storage(*addr, *key);
}

inline evmc_storage_status set_storage(evmc_host_context* h,
                                       const evmc_address* addr,
                                       const evmc_bytes32* key,
                                       const evmc_bytes32* value) noexcept
{
    return Host::from_context(h)->set_storage(*addr, *key, *value);
}

inline evmc_uint256be get_balance(evmc_host_context* h, const evmc_address* addr) noexcept
{
    return Host::from_context(h)->get_balance(*addr);
}

inline size_t get_code_size(evmc_host_context* h, const evmc_address* addr) noexcept
{
    return Host::from_context(h)->get_code_size(*addr);
}

inline evmc_bytes32 get_code_hash(evmc_host_context* h, const evmc_address* addr) noexcept
{
    return Host::from_context(h)->get_code_hash(*addr);
}

inline size_t copy_code(evmc_host_context* h,
                        const evmc_address* addr,
                        size_t code_offset,
                        uint8_t* buffer_data,
                        size_t buffer_size) noexcept
{
    return Host::from_context(h)->copy_code(*addr, code_offset, buffer_data, buffer_size);
}

inline void selfdestruct(evmc_host_context* h,
                         const evmc_address* addr,
                         const evmc_address* beneficiary) noexcept
{
    Host::from_context(h)->selfdestruct(*addr, *beneficiary);
}

inline evmc_result call(evmc_host_context* h, const evmc_message* msg) noexcept
{
    return Host::from_context(h)->call(*msg).release_raw();
}

inline evmc_tx_context get_tx_context(evmc_host_context* h) noexcept
{
    return Host::from_context(h)->get_tx_context();
}

inline evmc_bytes32 get_block_hash(evmc_host_context* h, int64_t block_number) noexcept
{
    return Host::from_context(h)->get_block_hash(block_number);
}

inline void emit_log(evmc_host_context* h,
                     const evmc_address* addr,
                     const uint8_t* data,
                     size_t data_size,
                     const evmc_bytes32 topics[],
                     size_t num_topics) noexcept
{
    Host::from_context(h)->emit_log(*addr, data, data_size, static_cast<const bytes32*>(topics),
                                    num_topics);
}
}  // namespace internal

inline const evmc_host_interface& Host::get_interface() noexcept
{
    static constexpr evmc_host_interface interface{
        ::evmc::internal::account_exists, ::evmc::internal::get_storage,
        ::evmc::internal::set_storage,    ::evmc::internal::get_balance,
        ::evmc::internal::get_code_size,  ::evmc::internal::get_code_hash,
        ::evmc::internal::copy_code,      ::evmc::internal::selfdestruct,
        ::evmc::internal::call,           ::evmc::internal::get_tx_context,
        ::evmc::internal::get_block_hash, ::evmc::internal::emit_log};
    return interface;
}
}  // namespace evmc


namespace std
{
/// Hash operator template specialization for evmc::address. Needed for unordered containers.
template <>
struct hash<evmc::address>
{
    /// Hash operator using FNV1a-based folding.
    constexpr size_t operator()(const evmc::address& s) const noexcept
    {
        using namespace evmc;
        using namespace fnv;
        return static_cast<size_t>(fnv1a_by64(
            fnv1a_by64(fnv1a_by64(fnv::offset_basis, load64be(&s.bytes[0])), load64be(&s.bytes[8])),
            load32be(&s.bytes[16])));
    }
};

/// Hash operator template specialization for evmc::bytes32. Needed for unordered containers.
template <>
struct hash<evmc::bytes32>
{
    /// Hash operator using FNV1a-based folding.
    constexpr size_t operator()(const evmc::bytes32& s) const noexcept
    {
        using namespace evmc;
        using namespace fnv;
        return static_cast<size_t>(
            fnv1a_by64(fnv1a_by64(fnv1a_by64(fnv1a_by64(fnv::offset_basis, load64be(&s.bytes[0])),
                                             load64be(&s.bytes[8])),
                                  load64be(&s.bytes[16])),
                       load64be(&s.bytes[24])));
    }
};
}  // namespace std
