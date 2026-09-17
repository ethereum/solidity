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
#include <string_view>
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

/// ethdebug/format/type: one of the known elementary or complex kinds. The
/// parts a type document is made of are nested here, so that they are
/// addressed as `Type::Array` and do not clash with the language types of the
/// frontend.
struct Type
{
	/// ethdebug/format/type/reference: a type known by its ID in the type resources.
	struct Reference
	{
		materials::ID id;
	};

	/// ethdebug/format/type/specifier: a full type representation or a reference.
	struct Specifier
	{
		Specifier(Reference _reference): value(std::move(_reference)) {}
		/// @a _type must not be null.
		explicit Specifier(std::shared_ptr<Type const> _type);

		std::variant<Reference, std::shared_ptr<Type const>> value;
	};

	/// ethdebug/format/type/wrapper: `{ "type": ... }`, with the `name` that struct
	/// member fields and tuple elements may carry.
	struct Wrapper
	{
		std::optional<std::string> name;
		Specifier type;
	};

	/// ethdebug/format/type/definition.
	struct Definition
	{
		/// At least one of @a _name and @a _location must be set.
		Definition(std::optional<std::string> _name, std::optional<materials::SourceRange> _location);

		std::optional<std::string> name;
		std::optional<materials::SourceRange> location;
	};

	/// @returns whether @a _bits is a width the numeric kinds allow: a multiple
	/// of 8 up to 256.
	static bool isValidWidth(unsigned _bits) { return _bits >= 8 && _bits <= 256 && _bits % 8 == 0; }
	/// @returns whether @a _places is a number of decimal places the fixed-point
	/// kinds allow: between 1 and 80.
	static bool isValidPlaces(unsigned _places) { return _places >= 1 && _places <= 80; }

	// Elementary kinds
	struct UInt
	{
		/// @a _bits must satisfy isValidWidth().
		explicit UInt(unsigned _bits);

		unsigned bits;
	};

	struct Int
	{
		/// @a _bits must satisfy isValidWidth().
		explicit Int(unsigned _bits);

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
		/// @a _bits must satisfy isValidWidth() and @a _places isValidPlaces().
		UFixed(unsigned _bits, unsigned _places);

		unsigned bits;
		unsigned places;
	};

	struct Fixed
	{
		/// @a _bits must satisfy isValidWidth() and @a _places isValidPlaces().
		Fixed(unsigned _bits, unsigned _places);

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

	std::variant<
		UInt, Int, Bool, Bytes, String, UFixed, Fixed, Address, Contract, Enum,
		Alias, Array, Mapping, Struct, Tuple, Function
	> value;
};

/// ethdebug/format/pointer: a region or a collection of pointers. The
/// expressions and the kinds of pointers are nested here, so that they are
/// addressed as `Pointer::Region` and do not clash with names in use elsewhere.
///
/// The constructors of the parts assert the constraints the schema puts on
/// them, so that a pointer cannot be assembled in a shape that does not
/// serialize.
struct Pointer
{
	struct Expression;
	using Operands = std::vector<Expression>;

	/// @returns whether @a _text follows the identifier grammar of
	/// ethdebug/format/pointer/identifier: `^[a-zA-Z_\-]+[a-zA-Z0-9$_\-]*$`.
	static bool isIdentifier(std::string_view _text);
	/// @returns whether @a _text names a region: an identifier or `$this`.
	static bool isRegionReference(std::string_view _text);

	/// An unsigned number or `0x`-prefixed hex string.
	struct Literal
	{
		data::Unsigned value;
	};

	/// The value of a variable bound by a scope definition, a list index or a
	/// template parameter.
	struct Variable
	{
		/// @a _identifier must satisfy isIdentifier().
		explicit Variable(std::string _identifier);

		std::string identifier;
	};

	enum class Constant { WordSize };

	/// `{ ".slot" | ".offset" | ".length": <region> }`, a property of a named
	/// region or of `$this`.
	struct Lookup
	{
		enum class Property { Slot, Offset, Length };

		/// @a _region must satisfy isRegionReference().
		Lookup(Property _property, std::string _region);

		Property property;
		std::string region;
	};

	/// `{ "$read": <region> }`, the raw bytes in a region.
	struct Read
	{
		/// @a _region must satisfy isRegionReference().
		explicit Read(std::string _region);

		std::string region;
	};

	struct Arithmetic
	{
		enum class Operator { Sum, Difference, Product, Quotient, Remainder };

		/// Difference, quotient and remainder take exactly two @a _operands.
		Arithmetic(Operator _op, Operands _operands);

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
		/// @a _size, if set, must be positive; @a _operand must not be null.
		Resize(std::optional<unsigned> _size, std::shared_ptr<Expression const> _operand);

		std::optional<unsigned> size;
		std::shared_ptr<Expression const> operand;
	};

	/// ethdebug/format/pointer/expression
	struct Expression
	{
		std::variant<Literal, Variable, Constant, Lookup, Read, Arithmetic, Keccak256, Concat, Resize> value;
	};

	enum class Location { Stack, Memory, Storage, Calldata, Returndata, Transient, Code };

	/// ethdebug/format/pointer/region. Stack, storage and transient regions are
	/// addressed by @a slot (offset and length within the slot optional); the
	/// byte-oriented locations by @a offset and @a length.
	struct Region
	{
		/// @a _name, if set, must satisfy isIdentifier(). A stack, storage or
		/// transient region must have a @a _slot; a memory, calldata, returndata or
		/// code region must have an @a _offset and a @a _length and no slot.
		Region(
			std::optional<std::string> _name,
			Location _location,
			std::optional<Expression> _slot,
			std::optional<Expression> _offset = std::nullopt,
			std::optional<Expression> _length = std::nullopt
		);

		std::optional<std::string> name;
		Location location;
		std::optional<Expression> slot;
		std::optional<Expression> offset;
		std::optional<Expression> length;
	};

	struct Group
	{
		/// @a _members must not be empty.
		explicit Group(std::vector<Pointer> _members);

		std::vector<Pointer> members;
	};

	struct List
	{
		/// @a _each must satisfy isIdentifier(); @a _is must not be null.
		List(Expression _count, std::string _each, std::shared_ptr<Pointer const> _is);

		Expression count;
		std::string each;
		std::shared_ptr<Pointer const> is;
	};

	struct Conditional
	{
		/// @a _then must not be null.
		Conditional(Expression _condition, std::shared_ptr<Pointer const> _then, std::shared_ptr<Pointer const> _otherwise);

		Expression condition;
		std::shared_ptr<Pointer const> then;
		std::shared_ptr<Pointer const> otherwise;
	};

	/// Definitions are ordered: each may reference the earlier ones.
	struct Scope
	{
		/// @a _definitions must not be empty and must define identifiers; @a _in
		/// must not be null.
		Scope(std::vector<std::pair<std::string, Expression>> _definitions, std::shared_ptr<Pointer const> _in);

		std::vector<std::pair<std::string, Expression>> definitions;
		std::shared_ptr<Pointer const> in;
	};

	struct TemplateReference
	{
		/// @a _name and the names in @a _yields must satisfy isIdentifier().
		explicit TemplateReference(std::string _name, std::vector<std::pair<std::string, std::string>> _yields = {});

		std::string name;
		std::vector<std::pair<std::string, std::string>> yields;
	};

	/// ethdebug/format/pointer/template: @a body in terms of the @a expect variables.
	struct Template
	{
		/// @a _expect must list identifiers; @a _body must not be null.
		Template(std::vector<std::string> _expect, std::shared_ptr<Pointer const> _body);

		std::vector<std::string> expect;
		std::shared_ptr<Pointer const> body;
	};

	struct Templates
	{
		/// @a _templates must be named by identifiers; @a _in must not be null.
		Templates(std::vector<std::pair<std::string, Template>> _templates, std::shared_ptr<Pointer const> _in);

		std::vector<std::pair<std::string, Template>> templates;
		std::shared_ptr<Pointer const> in;
	};

	std::variant<Region, Group, List, Conditional, Scope, TemplateReference, Templates> value;
};

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
	std::map<std::string, Type> types;
	std::map<std::string, Pointer::Template> pointers;
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

void to_json(Json& _json, Type::Reference const& _reference);
void to_json(Json& _json, Type::Specifier const& _specifier);
void to_json(Json& _json, Type::Wrapper const& _wrapper);
void to_json(Json& _json, Type::Definition const& _definition);
void to_json(Json& _json, Type const& _type);

void to_json(Json& _json, Pointer::Expression const& _expression);
void to_json(Json& _json, Pointer::Region const& _region);
void to_json(Json& _json, Pointer const& _pointer);
void to_json(Json& _json, Pointer::Template const& _template);

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
