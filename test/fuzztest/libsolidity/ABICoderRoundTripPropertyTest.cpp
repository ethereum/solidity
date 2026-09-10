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
 * Property test for the ABI coders. The generated contract builds a value of a random type tuple off a tape of
 * fuzzer bytes, and a raw `fallback` exchanges plain byte strings, so the harness never encodes anything itself.
 * Checked with and without the optimiser:
 *   - `renormalize`: encode(decode(encode(v))) == encode(v), byte for byte;
 *   - `roundTripEquals`: the round-tripped value equals v via EVM primitives. This catches idempotent information
 *     loss (e.g. an array length written one short) that the first property misses.
 */
#include <test/EVMHost.h>

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/OptimiserSettings.h>

#include <liblangutil/EVMVersion.h>
#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libyul/Exceptions.h>

#include <libsolutil/Common.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Numeric.h>
#include <libsolutil/Whiskers.h>

#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
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
constexpr std::uint32_t maxTupleComponents = 3;
constexpr std::uint32_t maxEnumMembers = 4;
/// Never empty: builders index the tape modulo its length.
constexpr std::uint32_t minTapeLength = 32;
constexpr std::uint32_t maxTapeLength = 1024;

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
	enum class Kind
	{
		Uint, Int, Address, Bool, FixedBytes, Bytes, String,
		Enum, Contract, UserDefined,
		FixedArray, DynArray, Struct
	};

	Kind kind{};
	/// Outside of validation, read these through the accessors below.
	std::uint32_t width = 0;
	std::vector<TypePointer> components;

	std::uint32_t bits() const { solAssert(kind == Kind::Uint || kind == Kind::Int); return width; }
	std::uint32_t byteWidth() const { solAssert(kind == Kind::FixedBytes); return width; }
	std::uint32_t length() const { solAssert(kind == Kind::FixedArray); return width; }
	std::uint32_t memberCount() const { solAssert(kind == Kind::Enum); return width; }
	AbiType const& element() const
	{
		solAssert(kind == Kind::FixedArray || kind == Kind::DynArray);
		return *components.front();
	}
	AbiType const& underlying() const { solAssert(kind == Kind::UserDefined); return *components.front(); }
	std::vector<TypePointer> const& fields() const { solAssert(kind == Kind::Struct); return components; }
};

/// Separate from `isValueType` so that `assertValidNode` cannot recurse into what it guards.
bool isElementaryValueKind(AbiType::Kind const _kind)
{
	switch (_kind)
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

void assertValidNode(AbiType const& _type)
{
	std::size_t const componentCount = _type.components.size();
	switch (_type.kind)
	{
	case AbiType::Kind::Uint:
	case AbiType::Kind::Int:
		solAssert(_type.width >= 8 && _type.width <= 256, "uintN/intN width out of range");
		solAssert(_type.width % 8 == 0, "uintN/intN width must be a multiple of 8");
		solAssert(componentCount == 0, "an integer type has no components");
		break;
	case AbiType::Kind::FixedBytes:
		solAssert(_type.width >= 1 && _type.width <= 32, "bytesN width out of range");
		solAssert(componentCount == 0, "bytesN has no components");
		break;
	case AbiType::Kind::Address:
	case AbiType::Kind::Bool:
	case AbiType::Kind::Bytes:
	case AbiType::Kind::String:
	case AbiType::Kind::Contract:
		solAssert(_type.width == 0, "this kind does not use width");
		solAssert(componentCount == 0, "this kind has no components");
		break;
	case AbiType::Kind::Enum:
		solAssert(_type.width >= 1 && _type.width <= 256, "an enum has between 1 and 256 members");
		solAssert(componentCount == 0, "an enum has no components");
		break;
	case AbiType::Kind::UserDefined:
		solAssert(_type.width == 0, "a user-defined value type does not use width");
		solAssert(componentCount == 1, "a user-defined value type wraps exactly one type");
		solAssert(
			isElementaryValueKind(_type.components.front()->kind),
			"Solidity only accepts an elementary value type as the underlying type"
		);
		break;
	case AbiType::Kind::FixedArray:
		solAssert(_type.width >= 1, "Solidity has no zero-length arrays");
		solAssert(componentCount == 1, "an array has exactly one element type");
		break;
	case AbiType::Kind::DynArray:
		solAssert(_type.width == 0, "a dynamic array has no length in its type");
		solAssert(componentCount == 1, "an array has exactly one element type");
		break;
	case AbiType::Kind::Struct:
		solAssert(_type.width == 0, "a struct does not use width");
		solAssert(componentCount >= 1, "Solidity requires a struct to have at least one member");
		break;
	}
}

void assertValidType(AbiType const& _type)
{
	assertValidNode(_type);
	for (TypePointer const& component: _type.components)
		assertValidType(*component);
}

TypePointer makeType(AbiType::Kind _kind, std::uint32_t _width = 0, std::vector<TypePointer> _components = {})
{
	AbiType type{_kind, _width, std::move(_components)};
	assertValidNode(type);
	return std::make_shared<AbiType const>(std::move(type));
}

bool isValueType(AbiType const& _type)
{
	assertValidNode(_type);
	switch (_type.kind)
	{
	case AbiType::Kind::Uint:
	case AbiType::Kind::Int:
	case AbiType::Kind::Address:
	case AbiType::Kind::Bool:
	case AbiType::Kind::FixedBytes:
	case AbiType::Kind::Enum:
	case AbiType::Kind::Contract:
	case AbiType::Kind::UserDefined:
		return true;
	default: return false;
	}
}

/// Structs are spelled out as tuples.
std::string signatureOf(AbiType const& _type)
{
	assertValidNode(_type);
	switch (_type.kind)
	{
	case AbiType::Kind::Uint: return "uint" + std::to_string(_type.bits());
	case AbiType::Kind::Int: return "int" + std::to_string(_type.bits());
	case AbiType::Kind::Address: return "address";
	case AbiType::Kind::Bool: return "bool";
	case AbiType::Kind::FixedBytes: return "bytes" + std::to_string(_type.byteWidth());
	case AbiType::Kind::Bytes: return "bytes";
	case AbiType::Kind::String: return "string";
	case AbiType::Kind::Enum: return "uint8";
	case AbiType::Kind::Contract: return "address";
	case AbiType::Kind::UserDefined: return signatureOf(_type.underlying());
	case AbiType::Kind::FixedArray: return signatureOf(_type.element()) + "[" + std::to_string(_type.length()) + "]";
	case AbiType::Kind::DynArray: return signatureOf(_type.element()) + "[]";
	case AbiType::Kind::Struct:
	{
		std::string fields;
		for (TypePointer const& field: _type.fields())
			fields += (fields.empty() ? "" : ",") + signatureOf(*field);
		return "(" + fields + ")";
	}
	}
	solAssert(false);
}

/// Otherwise FuzzTest's printer recurses into `AbiType`'s members and instantiates itself indefinitely.
template <typename Sink>
void AbslStringify(Sink& _sink, AbiType const& _type)
{
	_sink.Append(signatureOf(_type));
}

// ---------------------------------------------------------------------------------------------------------------
// Domains
// ---------------------------------------------------------------------------------------------------------------

std::vector<TypePointer> elementaryValueTypes()
{
	std::vector<TypePointer> types{
		makeType(AbiType::Kind::Address),
		makeType(AbiType::Kind::Bool),
	};
	for (std::uint32_t bytesWide = 1; bytesWide <= 32; ++bytesWide)
	{
		types.push_back(makeType(AbiType::Kind::Uint, 8 * bytesWide));
		types.push_back(makeType(AbiType::Kind::Int, 8 * bytesWide));
		types.push_back(makeType(AbiType::Kind::FixedBytes, bytesWide));
	}
	return types;
}

/// Encoded as `address` and `uint8`.
std::vector<TypePointer> enumAndContractTypes()
{
	std::vector<TypePointer> types{makeType(AbiType::Kind::Contract)};
	for (std::uint32_t members = 1; members <= maxEnumMembers; ++members)
		types.push_back(makeType(AbiType::Kind::Enum, members));
	return types;
}

std::vector<TypePointer> userDefinedValueTypes()
{
	std::vector<TypePointer> types;
	for (TypePointer const& underlying: elementaryValueTypes())
		types.push_back(makeType(AbiType::Kind::UserDefined, 0, {underlying}));
	return types;
}

/// Each group: 1/4 of draws
fuzztest::Domain<TypePointer> elementaryTypeDomain()
{
	return fuzztest::OneOf(
		fuzztest::ElementOf(elementaryValueTypes()),
		fuzztest::ElementOf(std::vector<TypePointer>{
			makeType(AbiType::Kind::Bytes),
			makeType(AbiType::Kind::String),
		}),
		fuzztest::ElementOf(enumAndContractTypes()),
		fuzztest::ElementOf(userDefinedValueTypes())
	);
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

struct TypedTape
{
	std::vector<TypePointer> types;
	bytes tape;
};

fuzztest::Domain<TypedTape> typedTapeDomain()
{
	return fuzztest::StructOf<TypedTape>(
		fuzztest::VectorOf(typeDomain(maxTypeDepth)).WithMinSize(1).WithMaxSize(maxTupleComponents),
		fuzztest::VectorOf(fuzztest::Arbitrary<uint8_t>()).WithMinSize(minTapeLength).WithMaxSize(maxTapeLength)
	);
}

// ---------------------------------------------------------------------------------------------------------------
// Solidity source generation
// ---------------------------------------------------------------------------------------------------------------

class TypeNamer
{
public:
	std::string name(AbiType const& _type)
	{
		switch (_type.kind)
		{
		case AbiType::Kind::FixedArray: return name(_type.element()) + "[" + std::to_string(_type.length()) + "]";
		case AbiType::Kind::DynArray: return name(_type.element()) + "[]";
		case AbiType::Kind::Struct: return declaredName(_type);
		case AbiType::Kind::Enum: return declaredName(_type);
		case AbiType::Kind::UserDefined: return declaredName(_type);
		case AbiType::Kind::Contract: return declaredName(_type);
		default: return signatureOf(_type);
		}
	}

	std::string declarations() const
	{
		std::string result;
		for (std::string const& declaration: m_declarations)
			result += declaration;
		return result;
	}

private:
	/// Emits the declaration on first use.
	std::string declaredName(AbiType const& _type)
	{
		if (auto const it = m_names.find(&_type); it != m_names.end())
			return it->second;

		// Components first, so nested declarations precede their users. Only for readable failure output.
		std::vector<std::string> componentNames;
		for (TypePointer const& component: _type.components)
			componentNames.push_back(name(*component));

		std::string const identifier = namePrefix(_type.kind) + std::to_string(m_declarations.size());
		m_names[&_type] = identifier;
		m_declarations.push_back(declaration(_type, identifier, componentNames));
		return identifier;
	}

	static std::string namePrefix(AbiType::Kind const _kind)
	{
		switch (_kind)
		{
		case AbiType::Kind::Struct: return "S";
		case AbiType::Kind::Enum: return "E";
		case AbiType::Kind::UserDefined: return "U";
		case AbiType::Kind::Contract: return "C";
		default: solAssert(false);
		}
	}

	static std::string declaration(
		AbiType const& _type,
		std::string const& _identifier,
		std::vector<std::string> const& _componentNames
	)
	{
		switch (_type.kind)
		{
		case AbiType::Kind::Struct:
		{
			std::string fields;
			for (std::size_t i = 0; i < _componentNames.size(); ++i)
				fields += "\t" + _componentNames[i] + " f" + std::to_string(i) + ";\n";
			return "struct " + _identifier + " {\n" + fields + "}\n";
		}
		case AbiType::Kind::Enum:
		{
			std::string members;
			for (std::uint32_t i = 0; i < _type.memberCount(); ++i)
				members += (i == 0 ? "" : ", ") + ("M" + std::to_string(i));
			return "enum " + _identifier + " { " + members + " }\n";
		}
		case AbiType::Kind::UserDefined:
			return "type " + _identifier + " is " + _componentNames.front() + ";\n";
		case AbiType::Kind::Contract:
			return "contract " + _identifier + " {}\n";
		default:
			solAssert(false);
		}
	}

	std::map<AbiType const*, std::string> m_names;
	std::vector<std::string> m_declarations;
};

std::string commaSeparated(std::vector<std::string> const& _parts)
{
	std::string result;
	for (std::string const& part: _parts)
		result += (result.empty() ? "" : ", ") + part;
	return result;
}

/// Value types must not carry one.
std::string location(AbiType const& _type, std::string const& _location)
{
	return isValueType(_type) ? "" : " " + _location;
}

/// e.g. "x0, x1"
std::string variableList(std::size_t const _count, std::string const& _prefix)
{
	std::vector<std::string> parts;
	for (std::size_t i = 0; i < _count; ++i)
		parts.push_back(_prefix + std::to_string(i));
	return commaSeparated(parts);
}

class ValueBuilder
{
public:
	explicit ValueBuilder(TypeNamer& _namer): m_namer(_namer) {}

	/// Emits the builder and those it calls on first use.
	std::string builder(AbiType const& _type)
	{
		// Keyed by declared name: structurally equal types can have distinct declarations.
		std::string const typeName = m_namer.name(_type);
		if (auto const it = m_builders.find(typeName); it != m_builders.end())
			return it->second;

		std::string const identifier = "build" + std::to_string(m_builders.size());
		m_builders[typeName] = identifier;
		m_definitions.push_back(definition(_type, typeName, identifier));
		return identifier;
	}

	std::string definitions() const
	{
		std::string result = tapeHelpers();
		for (std::string const& definition: m_definitions)
			result += definition;
		return result;
	}

private:
	/// `Tape` is passed by reference, so reads advance `pos` for the caller. Indexing wraps, so reads never fail.
	static std::string tapeHelpers()
	{
		return util::Whiskers(R"(
			struct Tape { bytes data; uint pos; }
			function readByte(Tape memory t) internal pure returns (uint8 b) {
				b = uint8(t.data[t.pos % t.data.length]);
				t.pos++;
			}
			function readWord(Tape memory t) internal pure returns (uint256 w) {
				for (uint i = 0; i < 32; i++)
					w = (w << 8) | readByte(t);
			}
			function readArrayLength(Tape memory t) internal pure returns (uint) {
				return readByte(t) % <arrayLengthCount>;
			}
			function readBytes(Tape memory t) internal pure returns (bytes memory r) {
				uint16[12] memory lengths = [uint16(0), 1, 2, 31, 32, 33, 63, 64, 65, 95, 96, 97];
				r = new bytes(lengths[readByte(t) % lengths.length]);
				for (uint i = 0; i < r.length; i++)
					r[i] = bytes1(readByte(t));
			}
		)")
			("arrayLengthCount", std::to_string(maxArrayLength + 1))
			.render();
	}

	std::string definition(AbiType const& _type, std::string const& _typeName, std::string const& _identifier)
	{
		std::string body;
		switch (_type.kind)
		{
		case AbiType::Kind::Bytes:
			body = "\t\tr = readBytes(t);\n";
			break;
		case AbiType::Kind::String:
			body = "\t\tr = string(readBytes(t));\n";
			break;
		case AbiType::Kind::UserDefined:
			body = "\t\tr = " + _typeName + ".wrap(" + builder(_type.underlying()) + "(t));\n";
			break;
		case AbiType::Kind::FixedArray:
		case AbiType::Kind::DynArray:
			if (_type.kind == AbiType::Kind::DynArray)
				body = "\t\tr = new " + _typeName + "(readArrayLength(t));\n";
			body +=
				"\t\tfor (uint i = 0; i < r.length; i++)\n"
				"\t\t\tr[i] = " + builder(_type.element()) + "(t);\n";
			break;
		case AbiType::Kind::Struct:
			// Not a constructor call: its argument evaluation order is unspecified.
			for (std::size_t i = 0; i < _type.fields().size(); ++i)
				body += "\t\tr.f" + std::to_string(i) + " = " + builder(*_type.fields()[i]) + "(t);\n";
			break;
		default:
			body =
				"\t\tuint256 w = readWord(t);\n"
				"\t\tr = " + valueExpression(_type, _typeName) + ";\n";
			break;
		}

		return
			"\tfunction " + _identifier + "(Tape memory t) internal pure returns (" +
				_typeName + location(_type, "memory") + " r) {\n" +
			body +
			"\t}\n";
	}

	static std::string valueExpression(AbiType const& _type, std::string const& _typeName)
	{
		switch (_type.kind)
		{
		case AbiType::Kind::Uint:
			return _type.bits() == 256 ? "w" : "uint" + std::to_string(_type.bits()) + "(w)";
		case AbiType::Kind::Int:
			return _type.bits() == 256 ? "int256(w)" : "int" + std::to_string(_type.bits()) + "(int256(w))";
		case AbiType::Kind::Address:
			return "address(uint160(w))";
		case AbiType::Kind::Bool:
			return "(w & 1) == 1";
		case AbiType::Kind::FixedBytes:
			return _type.byteWidth() == 32 ?
				"bytes32(w)" :
				"bytes" + std::to_string(_type.byteWidth()) + "(bytes32(w))";
		case AbiType::Kind::Enum:
			// Undeclared members make the decoder revert.
			return _typeName + "(uint8(w % " + std::to_string(_type.memberCount()) + "))";
		case AbiType::Kind::Contract:
			return _typeName + "(address(uint160(w)))";
		default:
			solAssert(false);
		}
	}

	TypeNamer& m_namer;
	std::map<std::string, std::string> m_builders;
	std::vector<std::string> m_definitions;
};

class EqualityChecker
{
public:
	explicit EqualityChecker(TypeNamer& _namer): m_namer(_namer) {}

	/// Emits the checker and those it calls on first use.
	std::string checker(AbiType const& _type)
	{
		std::string const typeName = m_namer.name(_type);
		if (auto const it = m_checkers.find(typeName); it != m_checkers.end())
			return it->second;

		std::string const identifier = "eq" + std::to_string(m_checkers.size());
		m_checkers[typeName] = identifier;
		m_definitions.push_back(definition(_type, typeName, identifier));
		return identifier;
	}

	std::string definitions() const
	{
		std::string result;
		for (std::string const& definition: m_definitions)
			result += definition;
		return result;
	}

private:
	std::string definition(AbiType const& _type, std::string const& _typeName, std::string const& _identifier)
	{
		std::string body;
		switch (_type.kind)
		{
		case AbiType::Kind::Bytes:
			body = "\t\treturn keccak256(a) == keccak256(b);\n";
			break;
		case AbiType::Kind::String:
			body = "\t\treturn keccak256(bytes(a)) == keccak256(bytes(b));\n";
			break;
		case AbiType::Kind::UserDefined:
			// No `==` unless one is attached.
			body = "\t\treturn " + _typeName + ".unwrap(a) == " + _typeName + ".unwrap(b);\n";
			break;
		case AbiType::Kind::FixedArray:
		case AbiType::Kind::DynArray:
			if (_type.kind == AbiType::Kind::DynArray)
				body = "\t\tif (a.length != b.length) return false;\n";
			body +=
				"\t\tfor (uint i = 0; i < a.length; i++)\n"
				"\t\t\tif (!" + checker(_type.element()) + "(a[i], b[i])) return false;\n"
				"\t\treturn true;\n";
			break;
		case AbiType::Kind::Struct:
		{
			for (std::size_t i = 0; i < _type.fields().size(); ++i)
			{
				std::string const field = ".f" + std::to_string(i);
				body +=
					"\t\tif (!" + checker(*_type.fields()[i]) + "(a" + field + ", b" + field + ")) return false;\n";
			}
			body += "\t\treturn true;\n";
			break;
		}
		default:
			body = "\t\treturn a == b;\n";
			break;
		}

		std::string const parameter = _typeName + location(_type, "memory");
		return
			"\tfunction " + _identifier + "(" + parameter + " a, " + parameter + " b) internal pure returns (bool) {\n" +
			body +
			"\t}\n";
	}

	TypeNamer& m_namer;
	std::map<std::string, std::string> m_checkers;
	std::vector<std::string> m_definitions;
};

/// Calldata arrives undecoded and the result leaves raw. The leading mode byte is this test's own convention.
std::string const rawDispatcher = R"(
	fallback(bytes calldata input) external returns (bytes memory) {
		bytes memory payload = input[1:];
		if (uint8(input[0]) == 0) return encodeValue(payload);
		if (uint8(input[0]) == 1) return renormalize(payload);
		if (roundTripEquals(payload)) return hex"01";
		return hex"00";
	}
)";

std::string const sourceHeader = "// SPDX-License-Identifier: GPL-3.0\npragma abicoder v2;\n";

class ContractGenerator
{
public:
	explicit ContractGenerator(std::vector<TypePointer> _types): m_types(std::move(_types)) {}

	StringMap memoryRoundTripSources()
	{
		// Everything is generated before `declarations()`/`definitions()` are read below.
		std::string const types = typeList();
		std::string const values = variableList(m_types.size(), "v");
		std::string const encodeValue = encodeValueFunction();
		std::string const roundTripEquals =
			roundTripEqualsFunction("pure", "abi.decode(abi.encode(" + values + "), (" + types + "))");
		std::string const decoded = variableDeclarations("memory", "v");

		return {
			{"types.sol", sourceHeader + m_namer.declarations()},
			{"C.sol",
				sourceHeader +
				"import \"types.sol\";\n"
				"contract C {\n" +
				m_builder.definitions() +
				m_checker.definitions() +
				encodeValue +
				roundTripEquals +
				"\tfunction renormalize(bytes memory input) internal pure returns (bytes memory) {\n"
				"\t\t(" + decoded + ") = abi.decode(input, (" + types + "));\n"
				"\t\treturn abi.encode(" + values + ");\n"
				"\t}\n" +
				rawDispatcher +
				"}\n"
			},
		};
	}

	StringMap callRoundTripSources()
	{
		std::string const encodeValue = encodeValueFunction();
		std::string const roundTripEquals =
			roundTripEqualsFunction("view", "callee.identity(" + variableList(m_types.size(), "v") + ")");
		std::string const parameters = variableDeclarations("calldata", "x");
		std::string const returnTypes = variableDeclarations("memory");
		std::string const decoded = variableDeclarations("memory", "v");
		std::string const results = variableDeclarations("memory", "r");
		std::string const types = typeList();

		return {
			{"types.sol", sourceHeader + m_namer.declarations()},
			{"callee.sol",
				sourceHeader +
				"import \"types.sol\";\n"
				"contract Callee {\n"
				"\tfunction identity(" + parameters + ")\n"
				"\t\texternal pure returns (" + returnTypes + ")\n"
				"\t{\n"
				"\t\treturn (" + variableList(m_types.size(), "x") + ");\n"
				"\t}\n"
				"}\n"
			},
			{"caller.sol",
				sourceHeader +
				"import \"types.sol\";\n"
				"import \"callee.sol\";\n"
				"contract C {\n"
				"\tCallee private callee;\n"
				"\tconstructor() { callee = new Callee(); }\n" +
				m_builder.definitions() +
				m_checker.definitions() +
				encodeValue +
				roundTripEquals +
				"\tfunction renormalize(bytes memory input) internal view returns (bytes memory) {\n"
				"\t\t(" + decoded + ") = abi.decode(input, (" + types + "));\n"
				"\t\t(" + results + ") = callee.identity(" + variableList(m_types.size(), "v") + ");\n"
				"\t\treturn abi.encode(" + variableList(m_types.size(), "r") + ");\n"
				"\t}\n" +
				rawDispatcher +
				"}\n"
			},
		};
	}

private:
	/// e.g. "S0, uint8[]"
	std::string typeList()
	{
		std::vector<std::string> parts;
		for (TypePointer const& type: m_types)
			parts.push_back(m_namer.name(*type));
		return commaSeparated(parts);
	}

	/// e.g. "S0 calldata x0, uint8 x1", or "S0 calldata, uint8" without @param _prefix.
	std::string variableDeclarations(std::string const& _location, std::string const& _prefix = "")
	{
		std::vector<std::string> parts;
		for (std::size_t i = 0; i < m_types.size(); ++i)
		{
			std::string const name = _prefix.empty() ? "" : " " + _prefix + std::to_string(i);
			parts.push_back(m_namer.name(*m_types[i]) + location(*m_types[i], _location) + name);
		}
		return commaSeparated(parts);
	}

	std::string buildTupleStatements()
	{
		std::string statements = "\t\tTape memory t = Tape(tape, 0);\n";
		for (std::size_t i = 0; i < m_types.size(); ++i)
			statements +=
				"\t\t" + m_namer.name(*m_types[i]) + location(*m_types[i], "memory") + " v" + std::to_string(i) +
				" = " + m_builder.builder(*m_types[i]) + "(t);\n";
		return statements;
	}

	std::string encodeValueFunction()
	{
		return
			"\tfunction encodeValue(bytes memory tape) internal pure returns (bytes memory) {\n" +
			buildTupleStatements() +
			"\t\treturn abi.encode(" + variableList(m_types.size(), "v") + ");\n"
			"\t}\n";
	}

	/// @param _transport the round trip under test, yielding `w0`..`wN`.
	std::string roundTripEqualsFunction(std::string const& _mutability, std::string const& _transport)
	{
		std::vector<std::string> comparisons;
		for (std::size_t i = 0; i < m_types.size(); ++i)
		{
			std::string const index = std::to_string(i);
			comparisons.push_back(m_checker.checker(*m_types[i]) + "(v" + index + ", w" + index + ")");
		}
		std::string conjunction;
		for (std::string const& comparison: comparisons)
			conjunction += (conjunction.empty() ? "" : " && ") + comparison;

		return
			"\tfunction roundTripEquals(bytes memory tape) internal " + _mutability + " returns (bool) {\n" +
			buildTupleStatements() +
			"\t\t(" + variableDeclarations("memory", "w") + ") = " + _transport + ";\n"
			"\t\treturn " + conjunction + ";\n"
			"\t}\n";
	}

	std::vector<TypePointer> m_types;
	TypeNamer m_namer;
	ValueBuilder m_builder{m_namer};
	EqualityChecker m_checker{m_namer};
};

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

struct CompilationResult
{
	bytes creationCode;
	std::string errors;
	/// A codegen limit unrelated to the ABI; the input is skipped.
	bool stackTooDeep = false;
	std::string stackTooDeepMessage;
};

CompilationResult compileContract(StringMap const& _sources, bool const _optimize)
{
	CompilerStack compiler;
	compiler.setSources(_sources);
	compiler.setViaIR(true);
	compiler.setOptimiserSettings(_optimize);

	try
	{
		if (!compiler.compile())
			return {{}, langutil::SourceReferenceFormatter::formatErrorInformation(compiler.errors(), compiler), false};
	}
	catch (yul::StackTooDeepError const& _error)
	{
		return {{}, {}, true, _error.comment() ? *_error.comment() : "Stack too deep."};
	}

	return {compiler.object("C").bytecode, {}, false};
}

/// Compilation dominates the runtime, and the fuzzer often mutates only the tape.
CompilationResult const& compileContractCached(StringMap const& _sources, bool const _optimize)
{
	static std::map<std::pair<StringMap, bool>, CompilationResult> cache;
	static constexpr std::size_t maxCacheSize = 512;

	auto key = std::make_pair(_sources, _optimize);
	if (auto const it = cache.find(key); it != cache.end())
		return it->second;
	if (cache.size() >= maxCacheSize)
		cache.clear();
	return cache.emplace(std::move(key), compileContract(_sources, _optimize)).first->second;
}

evmc_message baseMessage(bytes const& _input)
{
	evmc_message message = {};
	message.gas = std::numeric_limits<int64_t>::max();
	message.input_data = _input.data();
	message.input_size = _input.size();
	return message;
}

/// Must match `rawDispatcher`.
enum class Mode: std::uint8_t { EncodeValue = 0, Renormalize = 1, RoundTripEquals = 2 };

struct CallResult
{
	std::optional<bytes> returnValue;
	std::string failure;
};

CallResult callRaw(
	solidity::test::EVMHost& _host,
	evmc::address const& _address,
	Mode const _mode,
	bytes const& _payload
)
{
	bytes const input = bytes{static_cast<uint8_t>(_mode)} + _payload;
	evmc_message message = baseMessage(input);
	message.kind = EVMC_CALL;
	message.recipient = _address;
	message.code_address = _address;

	evmc::Result const result = _host.call(message);
	if (result.status_code != EVMC_SUCCESS)
		return {
			std::nullopt,
			"Call in mode " + std::to_string(static_cast<int>(_mode)) +
			" failed with status " + std::to_string(result.status_code)
		};

	return {bytes(result.output_data, result.output_data + result.output_size), {}};
}

struct RoundTripResult
{
	bytes encoded;
	/// `encoded`, decoded and encoded again.
	bytes renormalized;
	bool valuesEqual = false;
	std::string failure;
};

RoundTripResult runRoundTrip(bytes const& _creationCode, bytes const& _tape)
{
	char const* vmPath = getenv("ETH_EVMONE");
	evmc::VM& vm = solidity::test::EVMHost::getVM(vmPath ? vmPath : evmoneFilename);
	if (!vm)
		return {{}, {}, false, "Unable to load evmone. Set ETH_EVMONE or LD_LIBRARY_PATH."};

	solidity::test::EVMHost host(langutil::EVMVersion{}, vm);

	evmc_message createMessage = baseMessage(_creationCode);
	createMessage.kind = EVMC_CREATE;
	evmc::Result const createResult = host.call(createMessage);
	if (createResult.status_code != EVMC_SUCCESS)
		return {{}, {}, false, "Contract creation failed with status " + std::to_string(createResult.status_code)};

	CallResult encoded = callRaw(host, createResult.create_address, Mode::EncodeValue, _tape);
	if (!encoded.returnValue)
		return {{}, {}, false, encoded.failure};
	yulAssert(encoded.failure.empty());

	CallResult renormalized = callRaw(host, createResult.create_address, Mode::Renormalize, *encoded.returnValue);
	if (!renormalized.returnValue)
		return {*encoded.returnValue, {}, false, renormalized.failure};
	yulAssert(renormalized.failure.empty());

	CallResult const equal = callRaw(host, createResult.create_address, Mode::RoundTripEquals, _tape);
	if (!equal.returnValue)
		return {*encoded.returnValue, *renormalized.returnValue, false, equal.failure};
	if (equal.returnValue->size() != 1 || equal.returnValue->front() > 1)
		return {*encoded.returnValue, *renormalized.returnValue, false,
			"Internal error? Expected a single status byte, got " + util::toHex(*equal.returnValue)
		};
	yulAssert(equal.failure.empty());

	return {std::move(*encoded.returnValue), std::move(*renormalized.returnValue), equal.returnValue->front() == 1, {}};
}

// ---------------------------------------------------------------------------------------------------------------
// The property
// ---------------------------------------------------------------------------------------------------------------

void checkRoundTrip(StringMap const& _sources, bool const _optimize, TypedTape const& _typedTape)
{
	solAssert(!_typedTape.types.empty(), "the top-level tuple has at least one component");
	solAssert(!_typedTape.tape.empty(), "the builders index the tape modulo its length");
	for (TypePointer const& type: _typedTape.types)
		assertValidType(*type);

	std::vector<std::string> signatures;
	for (TypePointer const& type: _typedTape.types)
		signatures.push_back(signatureOf(*type));
	std::string const contract =
		"tuple: (" + commaSeparated(signatures) + ")\n" +
		"optimize: " + (_optimize ? "true" : "false") + "\n" +
		sourcesToString(_sources);
	std::string const context = contract + "tape: " + util::toHex(_typedTape.tape) + "\n";

	CompilationResult const& compilation = compileContractCached(_sources, _optimize);
	if (compilation.stackTooDeep)
	{
		static std::set<std::string> reported;
		if (reported.insert(contract).second)
			std::cerr
				<< "Skipping an input the via-IR code generator cannot compile.\n"
				<< compilation.stackTooDeepMessage << "\n"
				<< contract << std::endl;
		return;
	}
	ASSERT_TRUE(compilation.errors.empty()) << "Compilation failed.\n" << compilation.errors << context;

	RoundTripResult const execution = runRoundTrip(compilation.creationCode, _typedTape.tape);
	ASSERT_TRUE(execution.failure.empty()) << execution.failure << "\n" << context;

	ASSERT_EQ(util::toHex(execution.renormalized), util::toHex(execution.encoded)) << context;
	ASSERT_TRUE(execution.valuesEqual) << "The value did not survive the round trip.\n" << context;
}

}

void MemoryRoundTripIsIdentity(TypedTape const& _typedTape, bool const _optimize)
{
	checkRoundTrip(ContractGenerator(_typedTape.types).memoryRoundTripSources(), _optimize, _typedTape);
}

void CallRoundTripIsIdentity(TypedTape const& _typedTape, bool const _optimize)
{
	checkRoundTrip(ContractGenerator(_typedTape.types).callRoundTripSources(), _optimize, _typedTape);
}

TEST(ABICoderTypeInvariants, MalformedTypesAreRejected)
{
	using Kind = AbiType::Kind;
	// Bypasses `makeType`'s validation.
	auto raw = [](Kind _kind, std::uint32_t _width, std::vector<TypePointer> _components) {
		return AbiType{_kind, _width, std::move(_components)};
	};
	auto valid = [](Kind _kind, std::uint32_t _width = 0, std::vector<TypePointer> _components = {}) {
		return makeType(_kind, _width, std::move(_components));
	};

	EXPECT_ANY_THROW(assertValidNode(raw(Kind::Uint, 7, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::Uint, 264, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::Uint, 0, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::Uint, 8, {valid(Kind::Bool)})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::FixedBytes, 0, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::FixedBytes, 33, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::Bool, 1, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::Bytes, 0, {valid(Kind::Bool)})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::Enum, 0, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::Enum, 257, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::UserDefined, 0, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::UserDefined, 0, {valid(Kind::Bytes)})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::FixedArray, 0, {valid(Kind::Bool)})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::FixedArray, 2, {})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::DynArray, 3, {valid(Kind::Bool)})));
	EXPECT_ANY_THROW(assertValidNode(raw(Kind::Struct, 0, {})));
	EXPECT_ANY_THROW(makeType(Kind::Struct, 0, {}));

	AbiType const nested = raw(Kind::Struct, 0, {std::make_shared<AbiType const>(raw(Kind::Uint, 7, {}))});
	EXPECT_ANY_THROW(assertValidType(nested));

	EXPECT_NO_THROW(assertValidType(*valid(Kind::Struct, 0, {valid(Kind::Uint, 256), valid(Kind::Bytes)})));
	EXPECT_NO_THROW(assertValidType(*valid(Kind::UserDefined, 0, {valid(Kind::FixedBytes, 32)})));
}

FUZZ_TEST(ABICoderRoundTripProperty, MemoryRoundTripIsIdentity)
	.WithDomains(typedTapeDomain(), fuzztest::Arbitrary<bool>());

FUZZ_TEST(ABICoderRoundTripProperty, CallRoundTripIsIdentity)
	.WithDomains(typedTapeDomain(), fuzztest::Arbitrary<bool>());

}
