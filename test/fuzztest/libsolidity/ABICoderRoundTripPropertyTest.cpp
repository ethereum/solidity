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
 * Property test for the ABI coders: encoding a value, decoding that encoding and encoding the result again must
 * reproduce the first encoding byte for byte.
 *
 * A random tuple of types is drawn. The value itself is built inside the generated contract from a tape of raw
 * fuzzer bytes, so the test never spells out an ABI encoding of its own; the compiler is on both sides.
 *   - `MemoryRoundTripIsIdentity` runs the decoder and encoder in one source unit, with and without the optimiser;
 *   - `CallRoundTripIsIdentity` routes the value through an external call to a second source unit, which reaches
 *     the calldata decoder and the return-value encoder.
 *
 * For the tuple `(uint8, S0)` the generated source units are
 *
 *     // types.sol -- struct, enum and user-defined type declarations, shared by every other unit
 *     struct S0 {
 *         bytes f0;
 *     }
 *
 *     // C.sol
 *     pragma abicoder v2;
 *     import "types.sol";
 *     contract C {
 *         // ... tape helpers, then one builder per type ...
 *         function build1(bytes memory t, uint p) internal pure returns (S0 memory, uint) {
 *             (bytes memory f0, uint p0) = build2(t, p);
 *             return (S0(f0), p0);
 *         }
 *         function encodeValue(bytes memory tape) public pure returns (bytes memory) {
 *             uint8 v0;
 *             S0 memory v1;
 *             uint p = 0;
 *             (v0, p) = build0(tape, p);
 *             (v1, p) = build1(tape, p);
 *             return abi.encode(v0, v1);
 *         }
 *         function renormalize(bytes memory input) public pure returns (bytes memory) {
 *             (uint8 v0, S0 memory v1) = abi.decode(input, (uint8, S0));
 *             return abi.encode(v0, v1);
 *         }
 *     }
 *
 * Only the tuple's types reach the contract as source, so one compilation serves every tape drawn for a type.
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
#include <libsolutil/FunctionSelector.h>
#include <libsolutil/Numeric.h>

#include <fuzztest/fuzztest.h>
#include <gtest/gtest.h>

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
constexpr std::uint32_t maxTupleComponents = 3;
constexpr std::uint32_t maxEnumMembers = 4;
/// Raw entropy the generated builders read values off. Never empty, because they index it modulo its length.
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
	/// Bit width for Uint/Int, byte width for FixedBytes, element count for FixedArray, member count for Enum.
	/// Unused otherwise.
	std::uint32_t width = 0;
	/// Element type for arrays and underlying type for UserDefined (exactly one entry), field types for structs.
	std::vector<TypePointer> components;
};

/// The kinds that occupy a single word and take no components. Kept separate from `isValueType` so that the
/// invariant check below cannot recurse back into the functions it is guarding.
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

// Invariants for the datatypes in the test itself
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

/// Invariants for the datatypes in the test itself
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

/// Type in ABI signature notation, i.e. with structs spelled out as tuples.
std::string signatureOf(AbiType const& _type)
{
	assertValidNode(_type);
	switch (_type.kind)
	{
	case AbiType::Kind::Uint: return "uint" + std::to_string(_type.width);
	case AbiType::Kind::Int: return "int" + std::to_string(_type.width);
	case AbiType::Kind::Address: return "address";
	case AbiType::Kind::Bool: return "bool";
	case AbiType::Kind::FixedBytes: return "bytes" + std::to_string(_type.width);
	case AbiType::Kind::Bytes: return "bytes";
	case AbiType::Kind::String: return "string";
	case AbiType::Kind::Enum: return "uint8";
	case AbiType::Kind::Contract: return "address";
	case AbiType::Kind::UserDefined: return signatureOf(*_type.components.front());
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

/// FuzzTest picks a printer per type. Without this it would descend into `AbiType`'s members, and since those
/// are `AbiType` pointers again, the printer would instantiate itself indefinitely
template <typename Sink>
void AbslStringify(Sink& _sink, AbiType const& _type)
{
	_sink.Append(signatureOf(_type));
}

// ---------------------------------------------------------------------------------------------------------------
// Domains
// ---------------------------------------------------------------------------------------------------------------

/// Every elementary value type: all 32 widths of `uintN`/`intN`/`bytesN` plus `address` and `bool`
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

/// Contract and enum types, which Solidity spells differently but encodes as `address` and `uint8`.
std::vector<TypePointer> enumAndContractTypes()
{
	std::vector<TypePointer> types{makeType(AbiType::Kind::Contract)};
	for (std::uint32_t members = 1; members <= maxEnumMembers; ++members)
		types.push_back(makeType(AbiType::Kind::Enum, members));
	return types;
}

/// A user-defined value type wraps an elementary value type; those are the only underlying types Solidity accepts.
std::vector<TypePointer> userDefinedValueTypes()
{
	std::vector<TypePointer> types;
	for (TypePointer const& underlying: elementaryValueTypes())
		types.push_back(makeType(AbiType::Kind::UserDefined, 0, {underlying}));
	return types;
}

/// Each group gets a quarter of the draws
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

/// The components of the top-level tuple, and the tape the generated contract reads a value for them off. The
/// tuple is what the ABI actually specifies an encoding for; a single component is just its most common shape.
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

/// Renders Solidity type names, collecting the declarations the rendered names refer to along the way. Structs,
/// enums and user-defined value types each need one, and they all go into a single source unit that every generated
/// contract imports.
class TypeNamer
{
public:
	std::string name(AbiType const& _type)
	{
		switch (_type.kind)
		{
		case AbiType::Kind::FixedArray: return name(*_type.components.front()) + "[" + std::to_string(_type.width) + "]";
		case AbiType::Kind::DynArray: return name(*_type.components.front()) + "[]";
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
	/// @returns the name of the type's declaration, emitting the declaration on first use.
	std::string declaredName(AbiType const& _type)
	{
		if (auto const it = m_names.find(&_type); it != m_names.end())
			return it->second;

		// Everything the declaration refers to is named first, so that nested declarations precede the ones using
		// them. Solidity does not require that, but it keeps the generated source readable when a case fails.
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
			for (std::uint32_t i = 0; i < _type.width; ++i)
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

/// Data location suffix for a variable of the given type. Value types must not carry one.
std::string location(AbiType const& _type, std::string const& _location)
{
	return isValueType(_type) ? "" : " " + _location;
}

/// e.g. "S0, uint8[]" -- the type list `abi.decode` takes.
std::string typeList(TypeNamer& _namer, std::vector<TypePointer> const& _types)
{
	std::vector<std::string> parts;
	for (TypePointer const& type: _types)
		parts.push_back(_namer.name(*type));
	return commaSeparated(parts);
}

/// e.g. "S0 memory, uint8" -- the return type list of the callee.
std::string returnTypeList(TypeNamer& _namer, std::vector<TypePointer> const& _types)
{
	std::vector<std::string> parts;
	for (TypePointer const& type: _types)
		parts.push_back(_namer.name(*type) + location(*type, "memory"));
	return commaSeparated(parts);
}

/// e.g. "S0 calldata x0, uint8 x1" -- a parameter or local variable list.
std::string variableDeclarations(
	TypeNamer& _namer,
	std::vector<TypePointer> const& _types,
	std::string const& _location,
	std::string const& _prefix
)
{
	std::vector<std::string> parts;
	for (std::size_t i = 0; i < _types.size(); ++i)
		parts.push_back(_namer.name(*_types[i]) + location(*_types[i], _location) + " " + _prefix + std::to_string(i));
	return commaSeparated(parts);
}

/// e.g. "x0, x1" -- the variables declared by `variableDeclarations` with the same prefix.
std::string variableList(std::size_t const _count, std::string const& _prefix)
{
	std::vector<std::string> parts;
	for (std::size_t i = 0; i < _count; ++i)
		parts.push_back(_prefix + std::to_string(i));
	return commaSeparated(parts);
}

/// Read by every generated builder. `t` is the tape and `p` a cursor into it; the tape is indexed modulo its
/// length, so a builder can always read as much as it needs and never has to reject what it was given.
std::string const tapeHelpers =
	"\tfunction readByte(bytes memory t, uint p) internal pure returns (uint8, uint) {\n"
	"\t\treturn (uint8(t[p % t.length]), p + 1);\n"
	"\t}\n"
	"\tfunction readWord(bytes memory t, uint p) internal pure returns (uint256 w, uint) {\n"
	"\t\tfor (uint i = 0; i < 32; i++) {\n"
	"\t\t\tuint8 b;\n"
	"\t\t\t(b, p) = readByte(t, p);\n"
	"\t\t\tw = (w << 8) | b;\n"
	"\t\t}\n"
	"\t\treturn (w, p);\n"
	"\t}\n"
	"\tfunction readArrayLength(bytes memory t, uint p) internal pure returns (uint, uint) {\n"
	"\t\tuint8 b;\n"
	"\t\t(b, p) = readByte(t, p);\n"
	"\t\treturn (b % " + std::to_string(maxArrayLength + 1) + ", p);\n"
	"\t}\n"
	// The lengths around a word boundary are the ones where the padding and the tail offsets that follow have to
	// be got right, so they are drawn out of a table rather than uniformly.
	"\tfunction readBytes(bytes memory t, uint p) internal pure returns (bytes memory r, uint) {\n"
	"\t\tuint16[12] memory lengths = [uint16(0), 1, 2, 31, 32, 33, 63, 64, 65, 95, 96, 97];\n"
	"\t\tuint8 b;\n"
	"\t\t(b, p) = readByte(t, p);\n"
	"\t\tr = new bytes(lengths[b % 12]);\n"
	"\t\tfor (uint i = 0; i < r.length; i++) {\n"
	"\t\t\tuint8 v;\n"
	"\t\t\t(v, p) = readByte(t, p);\n"
	"\t\t\tr[i] = bytes1(v);\n"
	"\t\t}\n"
	"\t\treturn (r, p);\n"
	"\t}\n";

/// Emits one Solidity function per type, building a value of it off the tape.
class ValueBuilder
{
public:
	explicit ValueBuilder(TypeNamer& _namer): m_namer(_namer) {}

	/// @returns the name of the function building a value of @param _type, emitting it and every builder it calls
	/// on first use.
	std::string builder(AbiType const& _type)
	{
		// Two structurally equal types drawn separately get separate declarations, and the declared name is what
		// tells them apart.
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
		std::string result = tapeHelpers;
		for (std::string const& definition: m_definitions)
			result += definition;
		return result;
	}

private:
	std::string definition(AbiType const& _type, std::string const& _typeName, std::string const& _identifier)
	{
		std::string returnValue = _typeName + location(_type, "memory");
		std::string body;
		switch (_type.kind)
		{
		case AbiType::Kind::Bytes:
			body = "\t\treturn readBytes(t, p);\n";
			break;
		case AbiType::Kind::String:
			body =
				"\t\t(bytes memory b, uint q) = readBytes(t, p);\n"
				"\t\treturn (string(b), q);\n";
			break;
		case AbiType::Kind::UserDefined:
		{
			AbiType const& underlying = *_type.components.front();
			std::string const underlyingBuilder = builder(underlying);
			body =
				"\t\t(" + m_namer.name(underlying) + " u, uint q) = " + underlyingBuilder + "(t, p);\n"
				"\t\treturn (" + _typeName + ".wrap(u), q);\n";
			break;
		}
		case AbiType::Kind::FixedArray:
		case AbiType::Kind::DynArray:
		{
			std::string const elementBuilder = builder(*_type.components.front());
			std::string bound = std::to_string(_type.width);
			if (_type.kind == AbiType::Kind::DynArray)
			{
				bound = "n";
				body =
					"\t\tuint n;\n"
					"\t\t(n, p) = readArrayLength(t, p);\n"
					"\t\tr = new " + _typeName + "(n);\n";
			}
			returnValue += " r";
			body +=
				"\t\tfor (uint i = 0; i < " + bound + "; i++)\n"
				"\t\t\t(r[i], p) = " + elementBuilder + "(t, p);\n"
				"\t\treturn (r, p);\n";
			break;
		}
		case AbiType::Kind::Struct:
		{
			std::string fields;
			std::string cursor = "p";
			for (std::size_t i = 0; i < _type.components.size(); ++i)
			{
				AbiType const& field = *_type.components[i];
				std::string const fieldBuilder = builder(field);
				std::string const next = "p" + std::to_string(i);
				body +=
					"\t\t(" + m_namer.name(field) + location(field, "memory") + " f" + std::to_string(i) +
					", uint " + next + ") = " + fieldBuilder + "(t, " + cursor + ");\n";
				fields += (i == 0 ? "" : ", ") + ("f" + std::to_string(i));
				cursor = next;
			}
			body += "\t\treturn (" + _typeName + "(" + fields + "), " + cursor + ");\n";
			break;
		}
		default:
			body =
				"\t\t(uint256 w, uint q) = readWord(t, p);\n"
				"\t\treturn (" + valueExpression(_type, _typeName) + ", q);\n";
			break;
		}

		return
			"\tfunction " + _identifier + "(bytes memory t, uint p) internal pure returns (" + returnValue + ", uint) {\n" +
			body +
			"\t}\n";
	}

	/// Expression turning the word `w` into a value of an elementary type.
	static std::string valueExpression(AbiType const& _type, std::string const& _typeName)
	{
		switch (_type.kind)
		{
		case AbiType::Kind::Uint:
			return _type.width == 256 ? "w" : "uint" + std::to_string(_type.width) + "(w)";
		case AbiType::Kind::Int:
			return _type.width == 256 ? "int256(w)" : "int" + std::to_string(_type.width) + "(int256(w))";
		case AbiType::Kind::Address:
			return "address(uint160(w))";
		case AbiType::Kind::Bool:
			return "(w & 1) == 1";
		case AbiType::Kind::FixedBytes:
			return _type.width == 32 ? "bytes32(w)" : "bytes" + std::to_string(_type.width) + "(bytes32(w))";
		case AbiType::Kind::Enum:
			// Only declared members are valid values; anything else makes the decoder revert.
			return _typeName + "(uint8(w % " + std::to_string(_type.width) + "))";
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

/// The function every generated contract exposes as the source of the value: it builds the tuple off the tape and
/// hands back its encoding.
std::string encodeValueFunction(TypeNamer& _namer, ValueBuilder& _builder, std::vector<TypePointer> const& _types)
{
	std::string body;
	for (std::size_t i = 0; i < _types.size(); ++i)
		body += "\t\t" + _namer.name(*_types[i]) + location(*_types[i], "memory") + " v" + std::to_string(i) + ";\n";
	body += "\t\tuint p = 0;\n";
	for (std::size_t i = 0; i < _types.size(); ++i)
		body += "\t\t(v" + std::to_string(i) + ", p) = " + _builder.builder(*_types[i]) + "(tape, p);\n";

	return
		"\tfunction encodeValue(bytes memory tape) public pure returns (bytes memory) {\n" +
		body +
		"\t\treturn abi.encode(" + variableList(_types.size(), "v") + ");\n"
		"\t}\n";
}

std::string const sourceHeader = "// SPDX-License-Identifier: GPL-3.0\npragma abicoder v2;\n";

StringMap memoryRoundTripSources(std::vector<TypePointer> const& _types)
{
	TypeNamer namer;
	ValueBuilder builder(namer);
	std::string const encodeValue = encodeValueFunction(namer, builder, _types);
	std::string const decoded = variableDeclarations(namer, _types, "memory", "v");
	std::string const types = typeList(namer, _types);
	std::string const values = variableList(_types.size(), "v");

	return {
		{"types.sol", sourceHeader + namer.declarations()},
		{"C.sol",
			sourceHeader +
			"import \"types.sol\";\n"
			"contract C {\n" +
			builder.definitions() +
			encodeValue +
			"\tfunction renormalize(bytes memory input) public pure returns (bytes memory) {\n"
			"\t\t(" + decoded + ") = abi.decode(input, (" + types + "));\n"
			"\t\treturn abi.encode(" + values + ");\n"
			"\t}\n"
			"}\n"
		},
	};
}

StringMap callRoundTripSources(std::vector<TypePointer> const& _types)
{
	TypeNamer namer;
	ValueBuilder builder(namer);
	std::string const encodeValue = encodeValueFunction(namer, builder, _types);
	std::string const parameters = variableDeclarations(namer, _types, "calldata", "x");
	std::string const returnTypes = returnTypeList(namer, _types);
	std::string const decoded = variableDeclarations(namer, _types, "memory", "v");
	std::string const results = variableDeclarations(namer, _types, "memory", "r");
	std::string const types = typeList(namer, _types);

	return {
		// The declarations get their own source unit so that both sides of the call refer to the same types.
		{"types.sol", sourceHeader + namer.declarations()},
		{"callee.sol",
			sourceHeader +
			"import \"types.sol\";\n"
			"contract Callee {\n"
			"\tfunction identity(" + parameters + ")\n"
			"\t\texternal pure returns (" + returnTypes + ")\n"
			"\t{\n"
			"\t\treturn (" + variableList(_types.size(), "x") + ");\n"
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
			builder.definitions() +
			encodeValue +
			"\tfunction renormalize(bytes memory input) public view returns (bytes memory) {\n"
			"\t\t(" + decoded + ") = abi.decode(input, (" + types + "));\n"
			"\t\t(" + results + ") = callee.identity(" + variableList(_types.size(), "v") + ");\n"
			"\t\treturn abi.encode(" + variableList(_types.size(), "r") + ");\n"
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

struct CompilationResult
{
	bytes creationCode;
	std::string errors;
	/// A known limit of the code generator rather than anything to do with the ABI, so the input is skipped.
	bool stackTooDeep = false;
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
	catch (yul::StackTooDeepError const&)
	{
		return {{}, {}, true};
	}

	return {compiler.object("C").bytecode, {}, false};
}

/// Compilation dominates the runtime of this test and the fuzzer keeps the sources fixed while it mutates the
/// encoding, so results are memoised.
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

bytes zeroPadRight(bytes _data)
{
	if (_data.size() % 32 != 0)
		_data.resize(_data.size() + 32 - _data.size() % 32, 0);
	solAssert(_data.size() % 32 == 0);
	return _data;
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

struct CallResult
{
	std::optional<bytes> returnValue;
	std::string failure;
};

/// Calls a `function (bytes) returns (bytes)` on an already deployed contract.
CallResult callBytesFunction(
	solidity::test::EVMHost& _host,
	evmc::address const& _address,
	std::string const& _signature,
	bytes const& _argument
)
{
	bytes const input = util::selectorFromSignatureH32(_signature).asBytes() + encodeBytesArgument(_argument);
	evmc_message message = baseMessage(input);
	message.kind = EVMC_CALL;
	message.recipient = _address;
	message.code_address = _address;

	evmc::Result const result = _host.call(message);
	if (result.status_code != EVMC_SUCCESS)
		return {std::nullopt, _signature + " failed with status " + std::to_string(result.status_code)};

	bytes const returnData(result.output_data, result.output_data + result.output_size);
	std::optional<bytes> returnValue = decodeBytesReturnValue(returnData);
	if (!returnValue)
		return {std::nullopt, "Malformed `bytes` return value from " + _signature + ": " + util::toHex(returnData)};
	return {std::move(returnValue), {}};
}

struct RoundTripResult
{
	/// `abi.encode` of the value the contract built off the tape.
	bytes encoded;
	/// The same value encoded once more, after a decode of @a encoded.
	bytes renormalized;
	std::string failure;
};

RoundTripResult runRoundTrip(bytes const& _creationCode, bytes const& _tape)
{
	char const* vmPath = getenv("ETH_EVMONE");
	evmc::VM& vm = solidity::test::EVMHost::getVM(vmPath ? vmPath : evmoneFilename);
	if (!vm)
		return {{}, {}, "Unable to load evmone. Set ETH_EVMONE or LD_LIBRARY_PATH."};

	solidity::test::EVMHost host(langutil::EVMVersion{}, vm);

	evmc_message createMessage = baseMessage(_creationCode);
	createMessage.kind = EVMC_CREATE;
	evmc::Result const createResult = host.call(createMessage);
	if (createResult.status_code != EVMC_SUCCESS)
		return {{}, {}, "Contract creation failed with status " + std::to_string(createResult.status_code)};

	CallResult encoded = callBytesFunction(host, createResult.create_address, "encodeValue(bytes)", _tape);
	if (!encoded.returnValue)
		return {{}, {}, encoded.failure};

	CallResult renormalized = callBytesFunction(host, createResult.create_address, "renormalize(bytes)", *encoded.returnValue);
	if (!renormalized.returnValue)
		return {*encoded.returnValue, {}, renormalized.failure};

	return {std::move(*encoded.returnValue), std::move(*renormalized.returnValue), {}};
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
	std::string const context =
		"tuple: (" + commaSeparated(signatures) + ")\n" +
		"tape: " + util::toHex(_typedTape.tape) + "\n" +
		sourcesToString(_sources);

	CompilationResult const& compilation = compileContractCached(_sources, _optimize);
	if (compilation.stackTooDeep)
		return;
	ASSERT_TRUE(compilation.errors.empty()) << "Compilation failed.\n" << compilation.errors << context;

	RoundTripResult const execution = runRoundTrip(compilation.creationCode, _typedTape.tape);
	ASSERT_TRUE(execution.failure.empty()) << execution.failure << "\n" << context;

	ASSERT_EQ(util::toHex(execution.renormalized), util::toHex(execution.encoded)) << context;
}

}

void MemoryRoundTripIsIdentity(TypedTape const& _typedTape, bool const _optimize)
{
	checkRoundTrip(memoryRoundTripSources(_typedTape.types), _optimize, _typedTape);
}

void CallRoundTripIsIdentity(TypedTape const& _typedTape, bool const _optimize)
{
	checkRoundTrip(callRoundTripSources(_typedTape.types), _optimize, _typedTape);
}

/// A test for the property test itself
TEST(ABICoderTypeInvariants, MalformedTypesAreRejected)
{
	using Kind = AbiType::Kind;
	// Bypasses `makeType`, which validates on construction.
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
	// `bytes` is not a value type, so it cannot be an underlying type.
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

// Memory round trip identity
FUZZ_TEST(ABICoderRoundTripProperty, MemoryRoundTripIsIdentity)
	.WithDomains(typedTapeDomain(), fuzztest::Arbitrary<bool>());

// Call roundtrip identity
FUZZ_TEST(ABICoderRoundTripProperty, CallRoundTripIsIdentity)
	.WithDomains(typedTapeDomain(), fuzztest::Arbitrary<bool>());

}
