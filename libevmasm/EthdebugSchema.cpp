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

#include <libevmasm/EthdebugSchema.h>

#include <liblangutil/Exceptions.h>

#include <libsolutil/Numeric.h>
#include <libsolutil/Visitor.h>

#include <cctype>
#include <string_view>

using namespace solidity;
using namespace solidity::evmasm::ethdebug;

schema::Type::Specifier::Specifier(std::shared_ptr<Type const> _type): value(std::move(_type))
{
	solAssert(std::get<std::shared_ptr<Type const>>(value), "Type specifier without a type.");
}

schema::Type::Definition::Definition(std::optional<std::string> _name, std::optional<materials::SourceRange> _location):
	name(std::move(_name)),
	location(std::move(_location))
{
	solAssert(name || location, "Type definition without properties.");
}

schema::Type::UInt::UInt(unsigned _bits): bits(_bits)
{
	solAssert(isValidWidth(bits), "Type width must be a multiple of 8 bits up to 256.");
}

schema::Type::Int::Int(unsigned _bits): bits(_bits)
{
	solAssert(isValidWidth(bits), "Type width must be a multiple of 8 bits up to 256.");
}

schema::Type::UFixed::UFixed(unsigned _bits, unsigned _places): bits(_bits), places(_places)
{
	solAssert(isValidWidth(bits), "Type width must be a multiple of 8 bits up to 256.");
	solAssert(isValidPlaces(places), "Fixed point type must have between 1 and 80 decimal places.");
}

schema::Type::Fixed::Fixed(unsigned _bits, unsigned _places): bits(_bits), places(_places)
{
	solAssert(isValidWidth(bits), "Type width must be a multiple of 8 bits up to 256.");
	solAssert(isValidPlaces(places), "Fixed point type must have between 1 and 80 decimal places.");
}

bool schema::Pointer::isIdentifier(std::string_view _text)
{
	auto const isStart = [](char _c) { return std::isalpha(static_cast<unsigned char>(_c)) || _c == '_' || _c == '-'; };
	auto const isRest = [&](char _c) { return isStart(_c) || std::isdigit(static_cast<unsigned char>(_c)) || _c == '$'; };
	if (_text.empty() || !isStart(_text.front()))
		return false;
	for (char const c: _text)
		if (!isRest(c))
			return false;
	return true;
}

bool schema::Pointer::isRegionReference(std::string_view _text)
{
	return _text == "$this" || isIdentifier(_text);
}

schema::Pointer::Variable::Variable(std::string _identifier): identifier(std::move(_identifier))
{
	solAssert(isIdentifier(identifier), "Pointer expression variable \"" + identifier + "\" is not an identifier.");
}

schema::Pointer::Lookup::Lookup(Property _property, std::string _region): property(_property), region(std::move(_region))
{
	solAssert(isRegionReference(region), "Region reference \"" + region + "\" is not an identifier.");
}

schema::Pointer::Read::Read(std::string _region): region(std::move(_region))
{
	solAssert(isRegionReference(region), "Region reference \"" + region + "\" is not an identifier.");
}

schema::Pointer::Arithmetic::Arithmetic(Operator _op, Operands _operands): op(_op), operands(std::move(_operands))
{
	if (op == Operator::Difference || op == Operator::Quotient || op == Operator::Remainder)
		solAssert(operands.size() == 2, "Difference, quotient and remainder take exactly two operands.");
}

schema::Pointer::Resize::Resize(std::optional<unsigned> _size, std::shared_ptr<Expression const> _operand):
	size(_size),
	operand(std::move(_operand))
{
	solAssert(!size || *size > 0, "Resize expression needs a positive byte width.");
	solAssert(operand, "Resize expression without an operand.");
}

schema::Pointer::Region::Region(
	std::optional<std::string> _name,
	Location _location,
	std::optional<Expression> _slot,
	std::optional<Expression> _offset,
	std::optional<Expression> _length
):
	name(std::move(_name)),
	location(_location),
	slot(std::move(_slot)),
	offset(std::move(_offset)),
	length(std::move(_length))
{
	if (name)
		solAssert(isIdentifier(*name), "Region name \"" + *name + "\" is not an identifier.");
	switch (location)
	{
	case Location::Stack:
	case Location::Storage:
	case Location::Transient:
		// Word-oriented locations address by slot.
		solAssert(slot, "A stack, storage or transient region must address its slot.");
		break;
	case Location::Memory:
	case Location::Calldata:
	case Location::Returndata:
	case Location::Code:
		// Byte-oriented locations address by offset and length.
		solAssert(!slot && offset && length, "A memory, calldata, returndata or code region must address its offset and length.");
		break;
	}
}

schema::Pointer::Group::Group(std::vector<Pointer> _members): members(std::move(_members))
{
	solAssert(!members.empty(), "A group pointer must have at least one member.");
}

schema::Pointer::List::List(Expression _count, std::string _each, std::shared_ptr<Pointer const> _is):
	count(std::move(_count)),
	each(std::move(_each)),
	is(std::move(_is))
{
	solAssert(isIdentifier(each), "List index name \"" + each + "\" is not an identifier.");
	solAssert(is, "List element pointer is missing.");
}

schema::Pointer::Conditional::Conditional(
	Expression _condition,
	std::shared_ptr<Pointer const> _then,
	std::shared_ptr<Pointer const> _otherwise
):
	condition(std::move(_condition)),
	then(std::move(_then)),
	otherwise(std::move(_otherwise))
{
	solAssert(then, "Conditional consequent is missing.");
}

schema::Pointer::Scope::Scope(std::vector<std::pair<std::string, Expression>> _definitions, std::shared_ptr<Pointer const> _in):
	definitions(std::move(_definitions)),
	in(std::move(_in))
{
	solAssert(!definitions.empty(), "A scope pointer must define at least one variable.");
	for (auto const& [variable, expression]: definitions)
		solAssert(isIdentifier(variable), "Scope variable \"" + variable + "\" is not an identifier.");
	solAssert(in, "Scope target pointer is missing.");
}

schema::Pointer::TemplateReference::TemplateReference(std::string _name, std::vector<std::pair<std::string, std::string>> _yields):
	name(std::move(_name)),
	yields(std::move(_yields))
{
	solAssert(isIdentifier(name), "Template name \"" + name + "\" is not an identifier.");
	for (auto const& [producedName, newName]: yields)
	{
		solAssert(isIdentifier(producedName), "Yielded region name \"" + producedName + "\" is not an identifier.");
		solAssert(isIdentifier(newName), "Yielded region name \"" + newName + "\" is not an identifier.");
	}
}

schema::Pointer::Template::Template(std::vector<std::string> _expect, std::shared_ptr<Pointer const> _body):
	expect(std::move(_expect)),
	body(std::move(_body))
{
	for (std::string const& parameter: expect)
		solAssert(isIdentifier(parameter), "Template parameter \"" + parameter + "\" is not an identifier.");
	solAssert(body, "Pointer template without a body.");
}

schema::Pointer::Templates::Templates(std::vector<std::pair<std::string, Template>> _templates, std::shared_ptr<Pointer const> _in):
	templates(std::move(_templates)),
	in(std::move(_in))
{
	for (auto const& [templateName, definition]: templates)
		solAssert(isIdentifier(templateName), "Template name \"" + templateName + "\" is not an identifier.");
	solAssert(in, "Templates target pointer is missing.");
}


void schema::data::to_json(Json& _json, HexValue const& _hexValue)
{
	_json = util::toHex(_hexValue.value, util::HexPrefix::Add);
}

void schema::data::to_json(Json& _json, Unsigned const& _unsigned)
{
	std::visit(util::GenericVisitor{
		[&](HexValue const& _hexValue) { _json = _hexValue; },
		[&](std::uint64_t const _value) { _json = _value; }
	}, _unsigned.value);
}

void schema::materials::to_json(Json& _json, ID const& _id)
{
	std::visit(util::GenericVisitor{
		[&](std::string const& _hexValue) { _json = _hexValue; },
		[&](std::uint64_t const _value) { _json = _value; }
	}, _id.value);
}

void schema::materials::to_json(Json& _json, Reference const& _source)
{
	_json["id"] = _source.id;
	if (_source.type)
		_json["type"] = *_source.type == Reference::Type::Compilation ? "compilation" : "source";
}

void schema::materials::to_json(Json& _json, SourceRange::Range const& _range)
{
	_json["length"] = _range.length;
	_json["offset"] = _range.offset;
}


void schema::materials::to_json(Json& _json, SourceRange const& _sourceRange)
{
	_json["source"] = _sourceRange.source;
	if (_sourceRange.range)
		_json["range"] = *_sourceRange.range;
}

void schema::materials::to_json(Json& _json, Source const& _source)
{
	_json["id"] = _source.id;
	_json["path"] = _source.path;
	_json["contents"] = _source.contents;
	if (_source.encoding)
		_json["encoding"] = *_source.encoding;
	_json["language"] = _source.language;
}

void schema::materials::to_json(Json& _json, Compilation::Compiler const& _compiler)
{
	_json["name"] = _compiler.name;
	_json["version"] = _compiler.version;
}

void schema::materials::to_json(Json& _json, Compilation const& _compilation)
{
	_json["id"] = _compilation.id;
	_json["compiler"] = _compilation.compiler;
	if (_compilation.settings)
		_json["settings"] = *_compilation.settings;
	_json["sources"] = _compilation.sources;
}

void schema::to_json(Json& _json, Type::Reference const& _reference)
{
	_json = Json::object();
	_json["id"] = _reference.id;
}

void schema::to_json(Json& _json, Type::Specifier const& _specifier)
{
	std::visit(util::GenericVisitor{
		[&](Type::Reference const& _reference) { _json = _reference; },
		[&](std::shared_ptr<Type const> const& _type) { _json = *_type; }
	}, _specifier.value);
}

void schema::to_json(Json& _json, Type::Wrapper const& _wrapper)
{
	_json = Json::object();
	if (_wrapper.name)
		_json["name"] = *_wrapper.name;
	_json["type"] = _wrapper.type;
}

void schema::to_json(Json& _json, Type::Definition const& _definition)
{
	_json = Json::object();
	if (_definition.name)
		_json["name"] = *_definition.name;
	if (_definition.location)
		_json["location"] = *_definition.location;
}

void schema::to_json(Json& _json, Type const& _type)
{
	_json = Json::object();
	auto const definition = [&](std::optional<Type::Definition> const& _definition) {
		if (_definition)
			_json["definition"] = *_definition;
	};
	std::visit(util::GenericVisitor{
		[&](Type::UInt const& _uint)
		{
			_json["kind"] = "uint";
			_json["bits"] = _uint.bits;
		},
		[&](Type::Int const& _int)
		{
			_json["kind"] = "int";
			_json["bits"] = _int.bits;
		},
		[&](Type::Bool const&) { _json["kind"] = "bool"; },
		[&](Type::Bytes const& _bytes)
		{
			_json["kind"] = "bytes";
			if (_bytes.size)
				_json["size"] = *_bytes.size;
		},
		[&](Type::String const& _string)
		{
			_json["kind"] = "string";
			if (_string.encoding)
				_json["encoding"] = *_string.encoding;
		},
		[&](Type::UFixed const& _ufixed)
		{
			_json["kind"] = "ufixed";
			_json["bits"] = _ufixed.bits;
			_json["places"] = _ufixed.places;
		},
		[&](Type::Fixed const& _fixed)
		{
			_json["kind"] = "fixed";
			_json["bits"] = _fixed.bits;
			_json["places"] = _fixed.places;
		},
		[&](Type::Address const& _address)
		{
			_json["kind"] = "address";
			if (_address.payable)
				_json["payable"] = *_address.payable;
		},
		[&](Type::Contract const& _contract)
		{
			_json["kind"] = "contract";
			if (_contract.payable)
				_json["payable"] = *_contract.payable;
			if (_contract.kind == Type::Contract::Kind::Library)
				_json["library"] = true;
			else if (_contract.kind == Type::Contract::Kind::Interface)
				_json["interface"] = true;
			definition(_contract.definition);
		},
		[&](Type::Enum const& _enum)
		{
			_json["kind"] = "enum";
			_json["values"] = _enum.values;
			definition(_enum.definition);
		},
		[&](Type::Alias const& _alias)
		{
			_json["kind"] = "alias";
			_json["contains"] = _alias.contains;
			definition(_alias.definition);
		},
		[&](Type::Array const& _array)
		{
			_json["kind"] = "array";
			_json["contains"] = _array.contains;
			if (_array.count)
				_json["count"] = *_array.count;
		},
		[&](Type::Mapping const& _mapping)
		{
			_json["kind"] = "mapping";
			_json["contains"] = Json{{"key", _mapping.key}, {"value", _mapping.value}};
		},
		[&](Type::Struct const& _struct)
		{
			_json["kind"] = "struct";
			_json["contains"] = _struct.contains;
			definition(_struct.definition);
		},
		[&](Type::Tuple const& _tuple)
		{
			_json["kind"] = "tuple";
			_json["contains"] = _tuple.contains;
		},
		[&](Type::Function const& _function)
		{
			_json["kind"] = "function";
			_json[_function.visibility == Type::Function::Visibility::Internal ? "internal" : "external"] = true;
			Json contains{{"parameters", _function.parameters}};
			if (_function.returns)
				contains["returns"] = *_function.returns;
			_json["contains"] = std::move(contains);
			definition(_function.definition);
		}
	}, _type.value);
}

void schema::to_json(Json& _json, Pointer::Expression const& _expression)
{
	std::visit(util::GenericVisitor{
		[&](Pointer::Literal const& _literal) { _json = _literal.value; },
		[&](Pointer::Variable const& _variable) { _json = _variable.identifier; },
		[&](Pointer::Constant const _constant)
		{
			solAssert(_constant == Pointer::Constant::WordSize, "Unknown pointer expression constant.");
			_json = "$wordsize";
		},
		[&](Pointer::Lookup const& _lookup)
		{
			char const* property = nullptr;
			switch (_lookup.property)
			{
			case Pointer::Lookup::Property::Slot: property = ".slot"; break;
			case Pointer::Lookup::Property::Offset: property = ".offset"; break;
			case Pointer::Lookup::Property::Length: property = ".length"; break;
			}
			_json = Json{{property, _lookup.region}};
		},
		[&](Pointer::Read const& _read) { _json = Json{{"$read", _read.region}}; },
		[&](Pointer::Arithmetic const& _arithmetic)
		{
			switch (_arithmetic.op)
			{
			case Pointer::Arithmetic::Operator::Sum: _json = Json{{"$sum", _arithmetic.operands}}; break;
			case Pointer::Arithmetic::Operator::Product: _json = Json{{"$product", _arithmetic.operands}}; break;
			case Pointer::Arithmetic::Operator::Difference: _json = Json{{"$difference", _arithmetic.operands}}; break;
			case Pointer::Arithmetic::Operator::Quotient: _json = Json{{"$quotient", _arithmetic.operands}}; break;
			case Pointer::Arithmetic::Operator::Remainder: _json = Json{{"$remainder", _arithmetic.operands}}; break;
			}
		},
		[&](Pointer::Keccak256 const& _keccak256) { _json = Json{{"$keccak256", _keccak256.operands}}; },
		[&](Pointer::Concat const& _concat) { _json = Json{{"$concat", _concat.operands}}; },
		[&](Pointer::Resize const& _resize)
		{
			if (_resize.size)
				_json = Json{{"$sized" + std::to_string(*_resize.size), *_resize.operand}};
			else
				_json = Json{{"$wordsized", *_resize.operand}};
		}
	}, _expression.value);
}

void schema::to_json(Json& _json, Pointer::Region const& _region)
{
	_json = Json::object();
	if (_region.name)
		_json["name"] = *_region.name;
	char const* location = nullptr;
	switch (_region.location)
	{
	case Pointer::Location::Stack: location = "stack"; break;
	case Pointer::Location::Storage: location = "storage"; break;
	case Pointer::Location::Transient: location = "transient"; break;
	case Pointer::Location::Memory: location = "memory"; break;
	case Pointer::Location::Calldata: location = "calldata"; break;
	case Pointer::Location::Returndata: location = "returndata"; break;
	case Pointer::Location::Code: location = "code"; break;
	}
	_json["location"] = location;
	if (_region.slot)
		_json["slot"] = *_region.slot;
	if (_region.offset)
		_json["offset"] = *_region.offset;
	if (_region.length)
		_json["length"] = *_region.length;
}

void schema::to_json(Json& _json, Pointer const& _pointer)
{
	std::visit(util::GenericVisitor{
		[&](Pointer::Region const& _region) { _json = _region; },
		[&](Pointer::Group const& _group) { _json = Json{{"group", _group.members}}; },
		[&](Pointer::List const& _list)
		{
			_json = Json{{"list", Json{
				{"count", _list.count},
				{"each", _list.each},
				{"is", *_list.is}
			}}};
		},
		[&](Pointer::Conditional const& _conditional)
		{
			_json = Json{{"if", _conditional.condition}, {"then", *_conditional.then}};
			if (_conditional.otherwise)
				_json["else"] = *_conditional.otherwise;
		},
		[&](Pointer::Scope const& _scope)
		{
			// Definitions are ordered while JSON object members are not, so each
			// definition becomes its own define/in level.
			Json inner = *_scope.in;
			for (auto definition = _scope.definitions.rbegin(); definition != _scope.definitions.rend(); ++definition)
				inner = Json{{"define", Json{{definition->first, definition->second}}}, {"in", std::move(inner)}};
			_json = std::move(inner);
		},
		[&](Pointer::TemplateReference const& _reference)
		{
			_json = Json{{"template", _reference.name}};
			if (!_reference.yields.empty())
			{
				Json yields = Json::object();
				for (auto const& [producedName, newName]: _reference.yields)
					yields[producedName] = newName;
				_json["yields"] = std::move(yields);
			}
		},
		[&](Pointer::Templates const& _templates)
		{
			Json templates = Json::object();
			for (auto const& [templateName, definition]: _templates.templates)
				templates[templateName] = definition;
			_json = Json{{"templates", std::move(templates)}, {"in", *_templates.in}};
		}
	}, _pointer.value);
}

void schema::to_json(Json& _json, Pointer::Template const& _template)
{
	_json = Json{{"expect", _template.expect}, {"for", *_template.body}};
}

void schema::to_json(Json& _json, Program::Contract const& _contract)
{
	if (_contract.name)
		_json["name"] = *_contract.name;
	_json["definition"] = _contract.definition;
}

void schema::program::to_json(Json& _json, Context::Variable const& _contextVariable)
{
	auto const numProperties =
		_contextVariable.identifier.has_value() +
		_contextVariable.declaration.has_value();
	solRequire(numProperties >= 1, EthdebugException, "Context variable has no properties.");
	if (_contextVariable.identifier)
	{
		solRequire(!_contextVariable.identifier->empty(), EthdebugException, "Variable identifier must not be empty.");
		_json["identifier"] = *_contextVariable.identifier;
	}
	if (_contextVariable.declaration)
		_json["declaration"] = *_contextVariable.declaration;
}

void schema::program::to_json(Json& _json, Context const& _context)
{
	solRequire(_context.code.has_value() + _context.remark.has_value() + _context.variables.has_value() >= 1, EthdebugException, "Context needs >=1 properties.");
	if (_context.code)
		_json["code"] = *_context.code;
	if (_context.variables)
	{
		solRequire(!_context.variables->empty(), EthdebugException, "Context variables must not be empty if provided.");
		_json["variables"] = *_context.variables;
	}
	if (_context.remark)
		_json["remark"] = *_context.remark;
}

void schema::program::to_json(Json& _json, Instruction::Operation const& _operation)
{
	_json = { {"mnemonic", _operation.mnemonic} };
	if (!_operation.arguments.empty())
		_json["arguments"] = _operation.arguments;
}

void schema::program::to_json(Json& _json, Instruction const& _instruction)
{
	_json["offset"] = _instruction.offset;
	if (_instruction.operation)
		_json["operation"] = *_instruction.operation;
	if (_instruction.context)
		_json["context"] = *_instruction.context;
}

void schema::to_json(Json& _json, Program const& _program)
{
	if (_program.compilation)
		_json["compilation"] = *_program.compilation;
	_json["contract"] = _program.contract;
	_json["environment"] = _program.environment;
	if (_program.context)
		_json["context"] = *_program.context;
	_json["instructions"] = _program.instructions;
}

void schema::to_json(Json& _json, Program::Environment const& _environment)
{
	switch (_environment)
	{
	case Program::Environment::CALL:
		_json = "call";
		break;
	case Program::Environment::CREATE:
		_json = "create";
		break;
	}
}

void schema::info::to_json(Json& _json, Resources const& _resources)
{
	_json["compilation"] = _resources.compilation;
	_json["types"] = Json::object();
	for (auto const& [id, type]: _resources.types)
		_json["types"][id] = type;
	_json["pointers"] = Json::object();
	for (auto const& [name, pointerTemplate]: _resources.pointers)
	{
		solAssert(Pointer::isIdentifier(name), "Pointer template name \"" + name + "\" is not an identifier.");
		_json["pointers"][name] = pointerTemplate;
	}
}
