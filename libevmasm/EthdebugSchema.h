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

#pragma once

#include <libsolutil/Common.h>
#include <libsolutil/JSON.h>

#include <concepts>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace solidity::evmasm::ethdebug::schema
{

struct EthdebugException: virtual util::Exception {};

namespace data
{

struct HexValue
{
	bytes value;
};

struct Unsigned
{
	template<std::unsigned_integral T>
	Unsigned(T const _value)
	{
		solRequire(static_cast<T>(_value) <= std::numeric_limits<std::uint64_t>::max(), EthdebugException, "Too large value.");
		value = static_cast<std::uint64_t>(_value);
	}
	template<std::signed_integral T>
	Unsigned(T const _value)
	{
		solRequire(_value >= 0, EthdebugException, "NonNegativeValue got negative value.");
		solRequire(static_cast<std::make_unsigned_t<T>>(_value) <= std::numeric_limits<std::uint64_t>::max(), EthdebugException, "Too large value.");
		value = static_cast<std::uint64_t>(_value);
	}
	Unsigned(HexValue&& _value): value(std::move(_value)) {}

	std::variant<std::uint64_t, HexValue> value;
};

}

namespace materials
{

struct ID
{
	std::variant<std::string, std::uint64_t> value;
};

struct Reference
{
	enum class Type { Compilation, Source };
	ID id;
	std::optional<Type> type;
};

struct SourceRange
{
	struct Range
	{
		data::Unsigned length;
		data::Unsigned offset;
	};

	Reference source;
	std::optional<Range> range;
};

struct Source
{
	ID id;
	std::string path;
	std::string contents;
	std::optional<std::string> encoding;
	std::string language;
};

struct Compilation
{
	struct Compiler
	{
		std::string name;
		std::string version;
	};

	ID id;
	Compiler compiler;
	std::optional<Json> settings;
	std::vector<Source> sources;
};

}

namespace type
{

struct Type;
using TypePtr = std::shared_ptr<Type const>;

/// ethdebug/format/type/reference: a type known by its ID in the type resources.
struct Reference
{
	materials::ID id;
};

/// ethdebug/format/type/specifier: a full type representation or a reference.
struct Specifier
{
	std::variant<Reference, TypePtr> value;
};

/// ethdebug/format/type/wrapper: `{ "type": ... }`, with the `name` that struct
/// member fields and tuple elements may carry.
struct Wrapper
{
	std::optional<std::string> name;
	Specifier type;
};

/// ethdebug/format/type/definition. At least one of the fields must be set.
struct Definition
{
	std::optional<std::string> name;
	std::optional<materials::SourceRange> location;
};

// Elementary kinds
struct UInt
{
	unsigned bits;
};

struct Int
{
	unsigned bits;
};

struct Bool
{
};

/// The dynamic bytes type when @a size is unset.
struct Bytes
{
	std::optional<data::Unsigned> size;
};

struct String
{
	std::optional<std::string> encoding;
};

struct UFixed
{
	unsigned bits;
	unsigned places;
};

struct Fixed
{
	unsigned bits;
	unsigned places;
};

/// @a payable unset means the payability is not known.
struct Address
{
	std::optional<bool> payable;
};

struct Contract
{
	enum class Kind { Contract, Library, Interface };

	Kind kind = Kind::Contract;
	std::optional<bool> payable;
	std::optional<Definition> definition;
};

struct Enum
{
	std::vector<std::string> values;
	std::optional<Definition> definition;
};

// Complex kinds
struct Alias
{
	Wrapper contains;
	std::optional<Definition> definition;
};

/// Dynamically sized when @a count is unset.
struct Array
{
	Wrapper contains;
	std::optional<data::Unsigned> count;
};

struct Mapping
{
	Wrapper key;
	Wrapper value;
};

struct Struct
{
	std::vector<Wrapper> contains;
	std::optional<Definition> definition;
};

struct Tuple
{
	std::vector<Wrapper> contains;
};

/// @a parameters wraps a tuple type; @a returns is either a tuple wrapper or
/// the wrapper of a single type.
struct Function
{
	enum class Visibility { Internal, External };

	Visibility visibility;
	Wrapper parameters;
	std::optional<Wrapper> returns;
	std::optional<Definition> definition;
};

/// ethdebug/format/type: one of the known elementary or complex kinds.
struct Type
{
	std::variant<
		UInt, Int, Bool, Bytes, String, UFixed, Fixed, Address, Contract, Enum,
		Alias, Array, Mapping, Struct, Tuple, Function
	> value;
};

}

namespace pointer
{

struct Expression;
using Operands = std::vector<Expression>;

/// An unsigned number or `0x`-prefixed hex string.
struct Literal
{
	data::Unsigned value;
};

/// The value of a variable bound by a scope definition, a list index or a
/// template parameter.
struct Variable
{
	std::string identifier;
};

enum class Constant { WordSize };

/// `{ ".slot" | ".offset" | ".length": <region> }`, a property of a named
/// region or of `$this`.
struct Lookup
{
	enum class Property { Slot, Offset, Length };

	Property property;
	std::string region;
};

/// `{ "$read": <region> }`, the raw bytes in a region.
struct Read
{
	std::string region;
};

/// Difference, quotient and remainder take exactly two operands.
struct Arithmetic
{
	enum class Operator { Sum, Difference, Product, Quotient, Remainder };

	Operator op;
	Operands operands;
};

struct Keccak256
{
	Operands operands;
};

struct Concat
{
	Operands operands;
};

/// `{ "$sized<N>": ... }` with @a size N, or `{ "$wordsized": ... }` when unset.
struct Resize
{
	std::optional<unsigned> size;
	std::shared_ptr<Expression const> operand;
};

/// ethdebug/format/pointer/expression
struct Expression
{
	std::variant<Literal, Variable, Constant, Lookup, Read, Arithmetic, Keccak256, Concat, Resize> value;
};

struct Pointer;
using PointerPtr = std::shared_ptr<Pointer const>;

enum class Location { Stack, Memory, Storage, Calldata, Returndata, Transient, Code };

/// ethdebug/format/pointer/region. Stack, storage and transient regions are
/// addressed by @a slot (offset and length within the slot optional); the
/// byte-oriented locations by @a offset and @a length.
struct Region
{
	std::optional<std::string> name;
	Location location;
	std::optional<Expression> slot;
	std::optional<Expression> offset;
	std::optional<Expression> length;
};

struct Group
{
	std::vector<Pointer> members;
};

struct List
{
	Expression count;
	std::string each;
	PointerPtr is;
};

struct Conditional
{
	Expression condition;
	PointerPtr then;
	PointerPtr otherwise;
};

/// Definitions are ordered: each may reference the earlier ones.
struct Scope
{
	std::vector<std::pair<std::string, Expression>> definitions;
	PointerPtr in;
};

struct TemplateReference
{
	std::string name;
	std::vector<std::pair<std::string, std::string>> yields;
};

/// ethdebug/format/pointer/template: @a body in terms of the @a expect variables.
struct Template
{
	std::vector<std::string> expect;
	PointerPtr body;
};

struct Templates
{
	std::vector<std::pair<std::string, Template>> templates;
	PointerPtr in;
};

/// ethdebug/format/pointer: a region or a collection of pointers.
struct Pointer
{
	std::variant<Region, Group, List, Conditional, Scope, TemplateReference, Templates> value;
};

}

namespace program
{

struct Context
{
	struct Variable
	{
		std::optional<std::string> identifier;
		std::optional<materials::SourceRange> declaration;
		// TODO: type
		// TODO: pointer according to ethdebug/format/spec/pointer
	};

	std::optional<materials::SourceRange> code;
	std::optional<std::vector<Variable>> variables;
	std::optional<std::string> remark;
};

struct Instruction
{
	struct Operation
	{
		std::string mnemonic;
		std::vector<data::Unsigned> arguments;
	};

	data::Unsigned offset;
	std::optional<Operation> operation;
	std::optional<Context> context;
};

}

struct Program
{
	enum class Environment
	{
		CALL, CREATE
	};

	struct Contract
	{
		std::optional<std::string> name;
		materials::SourceRange definition;
	};

	std::optional<materials::Reference> compilation;
	Contract contract;
	Environment environment;
	std::optional<program::Context> context;
	std::vector<program::Instruction> instructions;
};

namespace info
{

/// Type documents keyed by the producer's type ID and pointer templates keyed
/// by the producer's template name.
struct Resources
{
	materials::Compilation compilation;
	std::map<std::string, type::Type> types;
	std::map<std::string, pointer::Template> pointers;
};

}

namespace data
{
void to_json(Json& _json, HexValue const& _hexValue);
void to_json(Json& _json, Unsigned const& _unsigned);
}

namespace materials
{
void to_json(Json& _json, ID const& _id);
void to_json(Json& _json, Reference const& _source);
void to_json(Json& _json, SourceRange::Range const& _range);
void to_json(Json& _json, SourceRange const& _sourceRange);
void to_json(Json& _json, Source const& _source);
void to_json(Json& _json, Compilation::Compiler const& _compiler);
void to_json(Json& _json, Compilation const& _compilation);
}

namespace type
{
void to_json(Json& _json, Reference const& _reference);
void to_json(Json& _json, Specifier const& _specifier);
void to_json(Json& _json, Wrapper const& _wrapper);
void to_json(Json& _json, Definition const& _definition);
void to_json(Json& _json, Type const& _type);
}

namespace pointer
{
void to_json(Json& _json, Expression const& _expression);
void to_json(Json& _json, Region const& _region);
void to_json(Json& _json, Pointer const& _pointer);
void to_json(Json& _json, Template const& _template);
}

namespace program
{
void to_json(Json& _json, Context::Variable const& _contextVariable);
void to_json(Json& _json, Context const& _context);
void to_json(Json& _json, Instruction::Operation const& _operation);
void to_json(Json& _json, Instruction const& _instruction);
}

void to_json(Json& _json, Program::Contract const& _contract);
void to_json(Json& _json, Program::Environment const& _environment);
void to_json(Json& _json, Program const& _program);

namespace info
{
void to_json(Json& _json, Resources const& _resources);
}

}
