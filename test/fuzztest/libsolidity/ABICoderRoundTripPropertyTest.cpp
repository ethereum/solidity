/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0
/**
 * Property test for the ABI coders: decoding an encoded value and encoding it again must be the identity.
 *
 * A random ABI type is drawn together with the canonical encoding of a random value of that type. The encoding is
 * built here from the ABI specification and shares no code with the compiler, so it is an independent reference
 * rather than a second opinion from the implementation under test. It is then run through contracts generated for
 * the type, which must hand it back byte for byte:
 *   - `MemoryRoundTripIsIdentity` evaluates `abi.encode(abi.decode(input, (T)))` in a single source unit, i.e. the
 *     memory decoder followed by the memory encoder, across coder v1/v2, legacy/IR codegen and optimiser settings;
 *   - `CrossCoderCallRoundTripIsIdentity` additionally sends the decoded value through an external call to a second
 *     source unit, which may use the other coder version. That covers the calldata decoder and pairs a v1 encoder
 *     with a v2 decoder and the other way round.
 *
 * Only canonical encodings are generated. ABI coder v1 is known not to clean dirty higher-order bits of array
 * elements (https://github.com/argotorg/solidity/issues/14985); that needs non-canonical input to trigger and is
 * therefore out of scope here.
 */

#include <test/EVMHost.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/OptimiserSettings.h>

#include <libyul/Exceptions.h>

#include <libevmasm/Exceptions.h>

#include <liblangutil/EVMVersion.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/FunctionSelector.h>
#include <libsolutil/Numeric.h>

#include <range/v3/algorithm/any_of.hpp>

#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using namespace solidity;

namespace solidity::frontend::test
{

namespace
{

constexpr std::uint32_t maxTypeDepth = 3;
constexpr std::uint32_t maxArrayLength = 3;
constexpr std::uint32_t maxStructFields = 3;
constexpr std::uint32_t maxByteStringLength = 70;

#ifdef _WIN32
constexpr auto evmoneFilename = "evmone.dll";
#elif defined(__APPLE__)
constexpr auto evmoneFilename = "libevmone.dylib";
#else
constexpr auto evmoneFilename = "libevmone.so";
#endif

// ---------------------------------------------------------------------------------------------------------------
// Type model
// ---------------------------------------------------------------------------------------------------------------

struct AbiType;
using TypePointer = std::shared_ptr<AbiType const>;

struct AbiType
{
	enum class Kind { Uint, Int, Address, Bool, FixedBytes, Bytes, String, FixedArray, DynArray, Struct };

	Kind kind{};
	/// Bit width for Uint/Int, byte width for FixedBytes, element count for FixedArray. Unused otherwise.
	std::uint32_t width = 0;
	/// Element type for arrays (exactly one entry), field types for structs.
	std::vector<TypePointer> components;
};

TypePointer makeType(AbiType::Kind _kind, std::uint32_t _width = 0, std::vector<TypePointer> _components = {})
{
	return std::make_shared<AbiType const>(AbiType{_kind, _width, std::move(_components)});
}

bool isValueType(AbiType const& _type)
{
	switch (_type.kind)
	{
	case AbiType::Kind::Uint:
	case AbiType::Kind::Int:
	case AbiType::Kind::Address:
	case AbiType::Kind::Bool:
	case AbiType::Kind::FixedBytes:
		return true;
	default:
		return false;
	}
}

bool isDynamic(AbiType const& _type)
{
	switch (_type.kind)
	{
	case AbiType::Kind::Bytes:
	case AbiType::Kind::String:
	case AbiType::Kind::DynArray:
		return true;
	case AbiType::Kind::FixedArray:
		return isDynamic(*_type.components.front());
	case AbiType::Kind::Struct:
		return ranges::any_of(_type.components, [](TypePointer const& _field) { return isDynamic(*_field); });
	default:
		return false;
	}
}

/// Size of the type's entry in the head part of the enclosing tuple, in bytes.
std::size_t headSize(AbiType const& _type)
{
	if (isDynamic(_type))
		return 32;
	switch (_type.kind)
	{
	case AbiType::Kind::FixedArray:
		return _type.width * headSize(*_type.components.front());
	case AbiType::Kind::Struct:
	{
		std::size_t size = 0;
		for (TypePointer const& field: _type.components)
			size += headSize(*field);
		return size;
	}
	default:
		return 32;
	}
}

/// ABI coder v1 only handles elementary types plus a single level of array nesting over value types. Deeper types
/// are rejected either by `Type::fullEncodingType` or by the legacy encoder ("Nested memory arrays not yet
/// implemented here").
bool supportedByCoderV1(AbiType const& _type)
{
	switch (_type.kind)
	{
	case AbiType::Kind::FixedArray:
	case AbiType::Kind::DynArray:
		return isValueType(*_type.components.front());
	case AbiType::Kind::Struct:
		return false;
	default:
		return true;
	}
}

/// Type in ABI signature notation, i.e. with structs spelled out as tuples.
std::string signatureOf(AbiType const& _type)
{
	switch (_type.kind)
	{
	case AbiType::Kind::Uint: return "uint" + std::to_string(_type.width);
	case AbiType::Kind::Int: return "int" + std::to_string(_type.width);
	case AbiType::Kind::Address: return "address";
	case AbiType::Kind::Bool: return "bool";
	case AbiType::Kind::FixedBytes: return "bytes" + std::to_string(_type.width);
	case AbiType::Kind::Bytes: return "bytes";
	case AbiType::Kind::String: return "string";
	case AbiType::Kind::FixedArray: return signatureOf(*_type.components.front()) + "[" + std::to_string(_type.width) + "]";
	case AbiType::Kind::DynArray: return signatureOf(*_type.components.front()) + "[]";
	case AbiType::Kind::Struct:
	{
		std::string fields;
		for (TypePointer const& field: _type.components)
			fields += (fields.empty() ? "" : ",") + signatureOf(*field);
		return "(" + fields + ")";
	}
	}
	solAssert(false);
}

/// FuzzTest picks a printer per type. Without this one it would descend into `AbiType`'s members, and since those
/// are `AbiType` pointers again, the printer would instantiate itself indefinitely.
template <typename Sink>
void AbslStringify(Sink& _sink, AbiType const& _type)
{
	_sink.Append(signatureOf(_type));
}

// ---------------------------------------------------------------------------------------------------------------
// Canonical encoding
// ---------------------------------------------------------------------------------------------------------------

bytes zeroPadRight(bytes _data)
{
	if (_data.size() % 32 != 0)
		_data.resize(_data.size() + 32 - _data.size() % 32, 0);
	return _data;
}

/// Head/tail encoding of a sequence of values whose standalone encodings are already known, as specified for tuples.
bytes composeTuple(std::vector<TypePointer> const& _types, std::vector<bytes> const& _encodings)
{
	solAssert(_types.size() == _encodings.size());

	std::size_t headLength = 0;
	for (TypePointer const& type: _types)
		headLength += headSize(*type);

	bytes head;
	bytes tail;
	for (std::size_t i = 0; i < _types.size(); ++i)
		if (isDynamic(*_types[i]))
		{
			head += toBigEndian(u256(headLength + tail.size()));
			tail += _encodings[i];
		}
		else
			head += _encodings[i];

	return head + tail;
}

bytes composeArray(TypePointer const& _elementType, std::vector<bytes> const& _encodings)
{
	return composeTuple(std::vector<TypePointer>(_encodings.size(), _elementType), _encodings);
}

// ---------------------------------------------------------------------------------------------------------------
// Domains
// ---------------------------------------------------------------------------------------------------------------

fuzztest::Domain<TypePointer> elementaryTypeDomain()
{
	std::vector<TypePointer> types{
		makeType(AbiType::Kind::Address),
		makeType(AbiType::Kind::Bool),
		makeType(AbiType::Kind::Bytes),
		makeType(AbiType::Kind::String),
	};
	// A representative selection rather than all widths: the ones that are not a multiple of the word size exercise
	// cleanup, the extremes the boundaries.
	for (std::uint32_t const bits: std::array<std::uint32_t, 8>{8, 16, 24, 32, 64, 128, 200, 256})
	{
		types.push_back(makeType(AbiType::Kind::Uint, bits));
		types.push_back(makeType(AbiType::Kind::Int, bits));
	}
	for (std::uint32_t const width: std::array<std::uint32_t, 5>{1, 4, 20, 31, 32})
		types.push_back(makeType(AbiType::Kind::FixedBytes, width));
	return fuzztest::ElementOf(std::move(types));
}

fuzztest::Domain<TypePointer> typeDomain(std::uint32_t const _depth)
{
	if (_depth == 0)
		return elementaryTypeDomain();

	fuzztest::Domain<TypePointer> const elementDomain = typeDomain(_depth - 1);
	return fuzztest::OneOf(
		elementaryTypeDomain(),
		fuzztest::Map(
			[](TypePointer const& _element, std::uint32_t const _length) {
				return makeType(AbiType::Kind::FixedArray, _length, {_element});
			},
			elementDomain,
			fuzztest::InRange<std::uint32_t>(1, maxArrayLength)
		),
		fuzztest::Map(
			[](TypePointer const& _element) { return makeType(AbiType::Kind::DynArray, 0, {_element}); },
			elementDomain
		),
		fuzztest::Map(
			[](std::vector<TypePointer> const& _fields) { return makeType(AbiType::Kind::Struct, 0, _fields); },
			fuzztest::VectorOf(elementDomain).WithMinSize(1).WithMaxSize(maxStructFields)
		)
	);
}

u256 wordFromParts(std::array<uint64_t, 4> const& _parts)
{
	u256 result = 0;
	for (uint64_t const part: _parts)
		result = (result << 64) | part;
	return result;
}

/// Domain of the single-word encoding of an integer of the given width, sign-extended to the full word for `intN`.
fuzztest::Domain<bytes> wordDomain(std::uint32_t const _bits, bool const _signed)
{
	return fuzztest::Map(
		[_bits, _signed](std::array<uint64_t, 4> const& _parts) {
			u256 const mask = _bits == 256 ? ~u256(0) : (u256(1) << _bits) - 1;
			u256 value = wordFromParts(_parts) & mask;
			if (_signed && _bits < 256 && boost::multiprecision::bit_test(value, _bits - 1))
				value |= ~mask;
			return toBigEndian(value);
		},
		fuzztest::Arbitrary<std::array<uint64_t, 4>>()
	);
}

/// Domain of the canonical encoding of a value of @param _type, as it appears standalone (i.e. not yet placed in the
/// head/tail layout of an enclosing tuple).
fuzztest::Domain<bytes> encodingDomain(TypePointer const& _type);

/// Folds the per-field domains into a domain of encoding sequences, independently of how many fields there are.
fuzztest::Domain<std::vector<bytes>> fieldEncodingsDomain(std::vector<TypePointer> const& _fields)
{
	fuzztest::Domain<std::vector<bytes>> result = fuzztest::Just(std::vector<bytes>{});
	for (TypePointer const& field: _fields)
		result = fuzztest::Map(
			[](std::vector<bytes> _encodings, bytes _next) {
				_encodings.push_back(std::move(_next));
				return _encodings;
			},
			result,
			encodingDomain(field)
		);
	return result;
}

fuzztest::Domain<bytes> encodingDomain(TypePointer const& _type)
{
	switch (_type->kind)
	{
	case AbiType::Kind::Uint:
		return wordDomain(_type->width, false);
	case AbiType::Kind::Int:
		return wordDomain(_type->width, true);
	case AbiType::Kind::Address:
		return wordDomain(160, false);
	case AbiType::Kind::Bool:
		return fuzztest::Map(
			[](bool const _value) { return toBigEndian(u256(_value ? 1 : 0)); },
			fuzztest::Arbitrary<bool>()
		);
	case AbiType::Kind::FixedBytes:
		// bytesN is left-aligned in its word, unlike the numeric types.
		return fuzztest::Map(
			[](bytes const& _value) { return zeroPadRight(_value); },
			fuzztest::VectorOf(fuzztest::Arbitrary<uint8_t>()).WithSize(_type->width)
		);
	case AbiType::Kind::Bytes:
	case AbiType::Kind::String:
		return fuzztest::Map(
			[](bytes const& _value) { return toBigEndian(u256(_value.size())) + zeroPadRight(_value); },
			fuzztest::VectorOf(fuzztest::Arbitrary<uint8_t>()).WithMaxSize(maxByteStringLength)
		);
	case AbiType::Kind::FixedArray:
	{
		TypePointer const elementType = _type->components.front();
		return fuzztest::Map(
			[elementType](std::vector<bytes> const& _elements) { return composeArray(elementType, _elements); },
			fuzztest::VectorOf(encodingDomain(elementType)).WithSize(_type->width)
		);
	}
	case AbiType::Kind::DynArray:
	{
		TypePointer const elementType = _type->components.front();
		return fuzztest::Map(
			[elementType](std::vector<bytes> const& _elements) {
				return toBigEndian(u256(_elements.size())) + composeArray(elementType, _elements);
			},
			fuzztest::VectorOf(encodingDomain(elementType)).WithMaxSize(maxArrayLength)
		);
	}
	case AbiType::Kind::Struct:
	{
		std::vector<TypePointer> const fields = _type->components;
		return fuzztest::Map(
			[fields](std::vector<bytes> const& _encodings) { return composeTuple(fields, _encodings); },
			fieldEncodingsDomain(fields)
		);
	}
	}
	solAssert(false);
}

struct TypedValue
{
	TypePointer type;
	/// Canonical standalone encoding of a value of `type`.
	bytes encoding;
};

fuzztest::Domain<TypedValue> typedValueDomain()
{
	return fuzztest::FlatMap(
		[](TypePointer const& _type) {
			return fuzztest::StructOf<TypedValue>(fuzztest::Just(_type), encodingDomain(_type));
		},
		typeDomain(maxTypeDepth)
	);
}

// ---------------------------------------------------------------------------------------------------------------
// Solidity source generation
// ---------------------------------------------------------------------------------------------------------------

/// Renders Solidity type names, collecting the struct definitions the rendered names refer to along the way.
class TypeNamer
{
public:
	std::string name(AbiType const& _type)
	{
		switch (_type.kind)
		{
		case AbiType::Kind::FixedArray: return name(*_type.components.front()) + "[" + std::to_string(_type.width) + "]";
		case AbiType::Kind::DynArray: return name(*_type.components.front()) + "[]";
		case AbiType::Kind::Struct: return structName(_type);
		default: return signatureOf(_type);
		}
	}

	std::string definitions() const
	{
		std::string result;
		for (std::string const& definition: m_definitions)
			result += definition;
		return result;
	}

private:
	std::string structName(AbiType const& _type)
	{
		if (auto const it = m_structNames.find(&_type); it != m_structNames.end())
			return it->second;

		// The field types are named first so that nested struct definitions precede the ones referring to them.
		std::string fields;
		for (std::size_t i = 0; i < _type.components.size(); ++i)
			fields += "\t" + name(*_type.components[i]) + " f" + std::to_string(i) + ";\n";

		std::string const identifier = "S" + std::to_string(m_definitions.size());
		m_definitions.push_back("struct " + identifier + " {\n" + fields + "}\n");
		m_structNames[&_type] = identifier;
		return identifier;
	}

	std::map<AbiType const*, std::string> m_structNames;
	std::vector<std::string> m_definitions;
};

enum class Coder { V1, V2 };

std::string coderPragma(Coder const _coder)
{
	return _coder == Coder::V1 ? "pragma abicoder v1;\n" : "pragma abicoder v2;\n";
}

/// Data location suffix for a parameter of the given type. Value types must not carry one.
std::string location(AbiType const& _type, std::string const& _location)
{
	return isValueType(_type) ? "" : " " + _location;
}

std::string const licenseHeader = "// SPDX-License-Identifier: GPL-3.0\n";

StringMap memoryRoundTripSources(AbiType const& _type, Coder const _coder)
{
	TypeNamer namer;
	std::string const typeName = namer.name(_type);

	return {
		{"types.sol", licenseHeader + namer.definitions()},
		{"C.sol",
			licenseHeader +
			coderPragma(_coder) +
			"import \"types.sol\";\n"
			"contract C {\n"
			"\tfunction roundtrip(bytes memory input) public pure returns (bytes memory) {\n"
			"\t\treturn abi.encode(abi.decode(input, (" + typeName + ")));\n"
			"\t}\n"
			"}\n"
		},
	};
}

StringMap crossCoderCallSources(AbiType const& _type, Coder const _callerCoder, Coder const _calleeCoder)
{
	TypeNamer namer;
	std::string const typeName = namer.name(_type);

	return {
		// The struct definitions get their own source unit so that both sides of the call refer to the same types.
		{"types.sol", licenseHeader + namer.definitions()},
		{"callee.sol",
			licenseHeader +
			coderPragma(_calleeCoder) +
			"import \"types.sol\";\n"
			"contract Callee {\n"
			"\tfunction identity(" + typeName + location(_type, "calldata") + " x)\n"
			"\t\texternal pure returns (" + typeName + location(_type, "memory") + ")\n"
			"\t{\n"
			"\t\treturn x;\n"
			"\t}\n"
			"}\n"
		},
		{"caller.sol",
			licenseHeader +
			coderPragma(_callerCoder) +
			"import \"types.sol\";\n"
			"import \"callee.sol\";\n"
			"contract C {\n"
			"\tCallee private callee;\n"
			"\tconstructor() { callee = new Callee(); }\n"
			"\tfunction roundtrip(bytes memory input) public view returns (bytes memory) {\n"
			"\t\treturn abi.encode(callee.identity(abi.decode(input, (" + typeName + "))));\n"
			"\t}\n"
			"}\n"
		},
	};
}

std::string sourcesToString(StringMap const& _sources)
{
	std::string result;
	for (auto const& [name, content]: _sources)
		result += "==== " + name + " ====\n" + content;
	return result;
}

// ---------------------------------------------------------------------------------------------------------------
// Compilation and execution
// ---------------------------------------------------------------------------------------------------------------

struct CompilationSettings
{
	bool viaIR = false;
	bool optimize = false;

	auto operator<=>(CompilationSettings const&) const = default;
};

struct CompilationResult
{
	bytes creationCode;
	std::string errors;
	/// Set when the contract hits a codegen limitation that is unrelated to the ABI coders, i.e. running out of
	/// stack slots or an unimplemented legacy codegen path. Such a case says nothing about the round trip.
	bool unsupported = false;
};

CompilationResult compileContract(StringMap const& _sources, CompilationSettings const _settings)
{
	CompilerStack compiler;
	compiler.setSources(_sources);
	compiler.setViaIR(_settings.viaIR);
	compiler.setOptimiserSettings(_settings.optimize);

	try
	{
		if (!compiler.compile())
			return {{}, langutil::SourceReferenceFormatter::formatErrorInformation(compiler.errors(), compiler), false};
	}
	catch (langutil::StackTooDeepError const&) { return {{}, {}, true}; }
	catch (yul::StackTooDeepError const&) { return {{}, {}, true}; }
	catch (evmasm::StackTooDeepException const&) { return {{}, {}, true}; }
	catch (langutil::UnimplementedFeatureError const&) { return {{}, {}, true}; }

	return {compiler.object("C").bytecode, {}, false};
}

/// Compilation dominates the runtime of this test and the fuzzer keeps the sources fixed while it mutates the
/// encoding, so results are memoised.
CompilationResult const& compileContractCached(StringMap const& _sources, CompilationSettings const _settings)
{
	static std::map<std::pair<StringMap, CompilationSettings>, CompilationResult> cache;
	static constexpr std::size_t maxCacheSize = 512;

	auto key = std::make_pair(_sources, _settings);
	if (auto const it = cache.find(key); it != cache.end())
		return it->second;
	if (cache.size() >= maxCacheSize)
		cache.clear();
	return cache.emplace(std::move(key), compileContract(_sources, _settings)).first->second;
}

evmc_message baseMessage(bytes const& _input)
{
	evmc_message message = {};
	message.gas = std::numeric_limits<int64_t>::max();
	message.input_data = _input.data();
	message.input_size = _input.size();
	return message;
}

/// `abi.encode` of a single `bytes` argument.
bytes encodeBytesArgument(bytes const& _data)
{
	return toBigEndian(u256(32)) + toBigEndian(u256(_data.size())) + zeroPadRight(_data);
}

/// Inverse of `encodeBytesArgument`, accepting only the canonical encoding the compiler produces.
std::optional<bytes> decodeBytesReturnValue(bytes const& _returnData)
{
	if (_returnData.size() < 64)
		return std::nullopt;
	if (fromBigEndian<u256>(bytes(_returnData.begin(), _returnData.begin() + 32)) != 32)
		return std::nullopt;
	u256 const length = fromBigEndian<u256>(bytes(_returnData.begin() + 32, _returnData.begin() + 64));
	if (length > _returnData.size() - 64)
		return std::nullopt;
	return bytes(_returnData.begin() + 64, _returnData.begin() + 64 + static_cast<std::ptrdiff_t>(length));
}

struct ExecutionResult
{
	std::optional<bytes> returnValue;
	std::string failure;
};

ExecutionResult deployAndCallRoundTrip(bytes const& _creationCode, bytes const& _encodedArgument)
{
	char const* vmPath = getenv("ETH_EVMONE");
	evmc::VM& vm = solidity::test::EVMHost::getVM(vmPath ? vmPath : evmoneFilename);
	if (!vm)
		return {std::nullopt, "Unable to load evmone. Set ETH_EVMONE or LD_LIBRARY_PATH."};

	solidity::test::EVMHost host(langutil::EVMVersion{}, vm);

	evmc_message createMessage = baseMessage(_creationCode);
	createMessage.kind = EVMC_CREATE;
	evmc::Result const createResult = host.call(createMessage);
	if (createResult.status_code != EVMC_SUCCESS)
		return {std::nullopt, "Contract creation failed with status " + std::to_string(createResult.status_code)};

	bytes const input =
		util::selectorFromSignatureH32("roundtrip(bytes)").asBytes() +
		encodeBytesArgument(_encodedArgument);
	evmc_message callMessage = baseMessage(input);
	callMessage.kind = EVMC_CALL;
	callMessage.recipient = createResult.create_address;
	callMessage.code_address = createResult.create_address;
	evmc::Result const callResult = host.call(callMessage);
	if (callResult.status_code != EVMC_SUCCESS)
		return {std::nullopt, "Round trip call failed with status " + std::to_string(callResult.status_code)};

	bytes const returnData(callResult.output_data, callResult.output_data + callResult.output_size);
	std::optional<bytes> returnValue = decodeBytesReturnValue(returnData);
	if (!returnValue)
		return {std::nullopt, "Malformed `bytes` return value: " + util::toHex(returnData)};
	return {std::move(returnValue), {}};
}

// ---------------------------------------------------------------------------------------------------------------
// The property
// ---------------------------------------------------------------------------------------------------------------

void checkRoundTrip(StringMap const& _sources, CompilationSettings const _settings, TypedValue const& _typedValue)
{
	// What the contract receives is the encoding of the one-element tuple `(T)`, which is what `abi.encode(v)` and
	// hence the round trip is expected to produce again.
	bytes const argument = composeTuple({_typedValue.type}, {_typedValue.encoding});
	std::string const context =
		"type: " + signatureOf(*_typedValue.type) + "\n" +
		"argument: " + util::toHex(argument) + "\n" +
		sourcesToString(_sources);

	CompilationResult const& compilation = compileContractCached(_sources, _settings);
	if (compilation.unsupported)
		return;
	ASSERT_TRUE(compilation.errors.empty()) << "Compilation failed.\n" << compilation.errors << context;

	ExecutionResult const execution = deployAndCallRoundTrip(compilation.creationCode, argument);
	ASSERT_TRUE(execution.returnValue.has_value()) << execution.failure << "\n" << context;

	ASSERT_EQ(util::toHex(*execution.returnValue), util::toHex(argument)) << context;
}

}

void MemoryRoundTripIsIdentity(
	TypedValue const& _typedValue,
	Coder _coder,
	bool const _viaIR,
	bool const _optimize
)
{
	// The IR pipeline only supports coder v2, and so does any type v1 cannot express.
	if (_viaIR || !supportedByCoderV1(*_typedValue.type))
		_coder = Coder::V2;

	checkRoundTrip(memoryRoundTripSources(*_typedValue.type, _coder), {_viaIR, _optimize}, _typedValue);
}

FUZZ_TEST(ABICoderRoundTripProperty, MemoryRoundTripIsIdentity)
	.WithDomains(
		typedValueDomain(),
		fuzztest::ElementOf({Coder::V1, Coder::V2}),
		fuzztest::Arbitrary<bool>(),
		fuzztest::Arbitrary<bool>()
	);

void CrossCoderCallRoundTripIsIdentity(
	TypedValue const& _typedValue,
	Coder _callerCoder,
	Coder _calleeCoder,
	bool const _optimize
)
{
	// A v1 source unit cannot even name a type that needs v2, no matter which side of the call it is on.
	if (!supportedByCoderV1(*_typedValue.type))
		_callerCoder = _calleeCoder = Coder::V2;

	checkRoundTrip(
		crossCoderCallSources(*_typedValue.type, _callerCoder, _calleeCoder),
		{false, _optimize},
		_typedValue
	);
}

FUZZ_TEST(ABICoderRoundTripProperty, CrossCoderCallRoundTripIsIdentity)
	.WithDomains(
		typedValueDomain(),
		fuzztest::ElementOf({Coder::V1, Coder::V2}),
		fuzztest::ElementOf({Coder::V1, Coder::V2}),
		fuzztest::Arbitrary<bool>()
	);

}
