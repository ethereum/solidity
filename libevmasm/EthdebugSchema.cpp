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

#include <libsolutil/Numeric.h>
#include <libsolutil/Visitor.h>

#include <cctype>
#include <string_view>

using namespace solidity;
using namespace solidity::evmasm::ethdebug;

namespace
{

/// The identifier grammar of ethdebug/format/pointer/identifier:
/// `^[a-zA-Z_\-]+[a-zA-Z0-9$_\-]*$`.
bool isIdentifier(std::string_view _text)
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

void requireIdentifier(std::string_view _text, std::string_view _what)
{
	solRequire(isIdentifier(_text), schema::EthdebugException, std::string(_what) + " \"" + std::string(_text) + "\" is not an identifier.");
}

/// A region reference is an identifier or `$this`.
void requireRegionReference(std::string_view _text)
{
	solRequire(_text == "$this" || isIdentifier(_text), schema::EthdebugException, "Region reference \"" + std::string(_text) + "\" is not an identifier.");
}

void requireBits(unsigned _bits)
{
	solRequire(_bits >= 8 && _bits <= 256 && _bits % 8 == 0, schema::EthdebugException, "Type width must be a multiple of 8 bits up to 256.");
}

void requirePlaces(unsigned _places)
{
	solRequire(_places >= 1 && _places <= 80, schema::EthdebugException, "Fixed point type must have between 1 and 80 decimal places.");
}

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

void schema::type::to_json(Json& _json, Reference const& _reference)
{
	_json = Json::object();
	_json["id"] = _reference.id;
}

void schema::type::to_json(Json& _json, Specifier const& _specifier)
{
	std::visit(util::GenericVisitor{
		[&](Reference const& _reference) { _json = _reference; },
		[&](TypePtr const& _type)
		{
			solRequire(_type, EthdebugException, "Type specifier without a type.");
			_json = *_type;
		}
	}, _specifier.value);
}

void schema::type::to_json(Json& _json, Wrapper const& _wrapper)
{
	_json = Json::object();
	if (_wrapper.name)
		_json["name"] = *_wrapper.name;
	_json["type"] = _wrapper.type;
}

void schema::type::to_json(Json& _json, Definition const& _definition)
{
	solRequire(_definition.name || _definition.location, EthdebugException, "Type definition has no properties.");
	_json = Json::object();
	if (_definition.name)
		_json["name"] = *_definition.name;
	if (_definition.location)
		_json["location"] = *_definition.location;
}

void schema::type::to_json(Json& _json, Type const& _type)
{
	_json = Json::object();
	auto const definition = [&](std::optional<Definition> const& _definition) {
		if (_definition)
			_json["definition"] = *_definition;
	};
	std::visit(util::GenericVisitor{
		[&](UInt const& _uint)
		{
			requireBits(_uint.bits);
			_json["kind"] = "uint";
			_json["bits"] = _uint.bits;
		},
		[&](Int const& _int)
		{
			requireBits(_int.bits);
			_json["kind"] = "int";
			_json["bits"] = _int.bits;
		},
		[&](Bool const&) { _json["kind"] = "bool"; },
		[&](Bytes const& _bytes)
		{
			_json["kind"] = "bytes";
			if (_bytes.size)
				_json["size"] = *_bytes.size;
		},
		[&](String const& _string)
		{
			_json["kind"] = "string";
			if (_string.encoding)
				_json["encoding"] = *_string.encoding;
		},
		[&](UFixed const& _ufixed)
		{
			requireBits(_ufixed.bits);
			requirePlaces(_ufixed.places);
			_json["kind"] = "ufixed";
			_json["bits"] = _ufixed.bits;
			_json["places"] = _ufixed.places;
		},
		[&](Fixed const& _fixed)
		{
			requireBits(_fixed.bits);
			requirePlaces(_fixed.places);
			_json["kind"] = "fixed";
			_json["bits"] = _fixed.bits;
			_json["places"] = _fixed.places;
		},
		[&](Address const& _address)
		{
			_json["kind"] = "address";
			if (_address.payable)
				_json["payable"] = *_address.payable;
		},
		[&](Contract const& _contract)
		{
			_json["kind"] = "contract";
			if (_contract.payable)
				_json["payable"] = *_contract.payable;
			if (_contract.kind == Contract::Kind::Library)
				_json["library"] = true;
			else if (_contract.kind == Contract::Kind::Interface)
				_json["interface"] = true;
			definition(_contract.definition);
		},
		[&](Enum const& _enum)
		{
			_json["kind"] = "enum";
			_json["values"] = _enum.values;
			definition(_enum.definition);
		},
		[&](Alias const& _alias)
		{
			_json["kind"] = "alias";
			_json["contains"] = _alias.contains;
			definition(_alias.definition);
		},
		[&](Array const& _array)
		{
			_json["kind"] = "array";
			_json["contains"] = _array.contains;
			if (_array.count)
				_json["count"] = *_array.count;
		},
		[&](Mapping const& _mapping)
		{
			_json["kind"] = "mapping";
			_json["contains"] = Json{{"key", _mapping.key}, {"value", _mapping.value}};
		},
		[&](Struct const& _struct)
		{
			_json["kind"] = "struct";
			_json["contains"] = _struct.contains;
			definition(_struct.definition);
		},
		[&](Tuple const& _tuple)
		{
			_json["kind"] = "tuple";
			_json["contains"] = _tuple.contains;
		},
		[&](Function const& _function)
		{
			_json["kind"] = "function";
			_json[_function.visibility == Function::Visibility::Internal ? "internal" : "external"] = true;
			Json contains{{"parameters", _function.parameters}};
			if (_function.returns)
				contains["returns"] = *_function.returns;
			_json["contains"] = std::move(contains);
			definition(_function.definition);
		}
	}, _type.value);
}

void schema::pointer::to_json(Json& _json, Expression const& _expression)
{
	auto const operands = [](Operands const& _operands, std::optional<size_t> _arity = std::nullopt) {
		if (_arity)
			solRequire(_operands.size() == *_arity, EthdebugException, "Pointer expression has the wrong number of operands.");
		return Json(_operands);
	};
	std::visit(util::GenericVisitor{
		[&](Literal const& _literal) { _json = _literal.value; },
		[&](Variable const& _variable)
		{
			requireIdentifier(_variable.identifier, "Pointer expression variable");
			_json = _variable.identifier;
		},
		[&](Constant const _constant)
		{
			solRequire(_constant == Constant::WordSize, EthdebugException, "Unknown pointer expression constant.");
			_json = "$wordsize";
		},
		[&](Lookup const& _lookup)
		{
			requireRegionReference(_lookup.region);
			char const* property = nullptr;
			switch (_lookup.property)
			{
			case Lookup::Property::Slot: property = ".slot"; break;
			case Lookup::Property::Offset: property = ".offset"; break;
			case Lookup::Property::Length: property = ".length"; break;
			}
			_json = Json{{property, _lookup.region}};
		},
		[&](Read const& _read)
		{
			requireRegionReference(_read.region);
			_json = Json{{"$read", _read.region}};
		},
		[&](Arithmetic const& _arithmetic)
		{
			switch (_arithmetic.op)
			{
			case Arithmetic::Operator::Sum: _json = Json{{"$sum", operands(_arithmetic.operands)}}; break;
			case Arithmetic::Operator::Product: _json = Json{{"$product", operands(_arithmetic.operands)}}; break;
			case Arithmetic::Operator::Difference: _json = Json{{"$difference", operands(_arithmetic.operands, 2)}}; break;
			case Arithmetic::Operator::Quotient: _json = Json{{"$quotient", operands(_arithmetic.operands, 2)}}; break;
			case Arithmetic::Operator::Remainder: _json = Json{{"$remainder", operands(_arithmetic.operands, 2)}}; break;
			}
		},
		[&](Keccak256 const& _keccak256) { _json = Json{{"$keccak256", operands(_keccak256.operands)}}; },
		[&](Concat const& _concat) { _json = Json{{"$concat", operands(_concat.operands)}}; },
		[&](Resize const& _resize)
		{
			solRequire(_resize.operand, EthdebugException, "Resize expression without an operand.");
			if (_resize.size)
			{
				solRequire(*_resize.size > 0, EthdebugException, "Resize expression needs a positive byte width.");
				_json = Json{{"$sized" + std::to_string(*_resize.size), *_resize.operand}};
			}
			else
				_json = Json{{"$wordsized", *_resize.operand}};
		}
	}, _expression.value);
}

void schema::pointer::to_json(Json& _json, Region const& _region)
{
	_json = Json::object();
	if (_region.name)
	{
		requireIdentifier(*_region.name, "Region name");
		_json["name"] = *_region.name;
	}
	char const* location = nullptr;
	bool wordOriented = false;
	switch (_region.location)
	{
	case Location::Stack: location = "stack"; wordOriented = true; break;
	case Location::Storage: location = "storage"; wordOriented = true; break;
	case Location::Transient: location = "transient"; wordOriented = true; break;
	case Location::Memory: location = "memory"; break;
	case Location::Calldata: location = "calldata"; break;
	case Location::Returndata: location = "returndata"; break;
	case Location::Code: location = "code"; break;
	}
	_json["location"] = location;
	// Word-oriented locations address by slot, byte-oriented ones by offset and length.
	if (wordOriented)
	{
		solRequire(_region.slot, EthdebugException, "A stack, storage or transient region must address its slot.");
	}
	else
	{
		solRequire(!_region.slot && _region.offset && _region.length, EthdebugException, "A memory, calldata, returndata or code region must address its offset and length.");
	}
	if (_region.slot)
		_json["slot"] = *_region.slot;
	if (_region.offset)
		_json["offset"] = *_region.offset;
	if (_region.length)
		_json["length"] = *_region.length;
}

void schema::pointer::to_json(Json& _json, Pointer const& _pointer)
{
	auto const subPointer = [](PointerPtr const& _sub, std::string_view _what) -> Pointer const& {
		solRequire(_sub, EthdebugException, std::string(_what) + " is missing.");
		return *_sub;
	};
	std::visit(util::GenericVisitor{
		[&](Region const& _region) { _json = _region; },
		[&](Group const& _group)
		{
			solRequire(!_group.members.empty(), EthdebugException, "A group pointer must have at least one member.");
			_json = Json{{"group", _group.members}};
		},
		[&](List const& _list)
		{
			requireIdentifier(_list.each, "List index name");
			_json = Json{{"list", Json{
				{"count", _list.count},
				{"each", _list.each},
				{"is", subPointer(_list.is, "List element pointer")}
			}}};
		},
		[&](Conditional const& _conditional)
		{
			_json = Json{{"if", _conditional.condition}, {"then", subPointer(_conditional.then, "Conditional consequent")}};
			if (_conditional.otherwise)
				_json["else"] = *_conditional.otherwise;
		},
		[&](Scope const& _scope)
		{
			solRequire(!_scope.definitions.empty(), EthdebugException, "A scope pointer must define at least one variable.");
			// Definitions are ordered while JSON object members are not, so each
			// definition becomes its own define/in level.
			Json inner = subPointer(_scope.in, "Scope target pointer");
			for (auto definition = _scope.definitions.rbegin(); definition != _scope.definitions.rend(); ++definition)
			{
				requireIdentifier(definition->first, "Scope variable");
				inner = Json{{"define", Json{{definition->first, definition->second}}}, {"in", std::move(inner)}};
			}
			_json = std::move(inner);
		},
		[&](TemplateReference const& _reference)
		{
			requireIdentifier(_reference.name, "Template name");
			_json = Json{{"template", _reference.name}};
			if (!_reference.yields.empty())
			{
				Json yields = Json::object();
				for (auto const& [producedName, newName]: _reference.yields)
				{
					requireIdentifier(producedName, "Yielded region name");
					requireIdentifier(newName, "Yielded region name");
					yields[producedName] = newName;
				}
				_json["yields"] = std::move(yields);
			}
		},
		[&](Templates const& _templates)
		{
			Json templates = Json::object();
			for (auto const& [templateName, definition]: _templates.templates)
			{
				requireIdentifier(templateName, "Template name");
				templates[templateName] = definition;
			}
			_json = Json{{"templates", std::move(templates)}, {"in", subPointer(_templates.in, "Templates target pointer")}};
		}
	}, _pointer.value);
}

void schema::pointer::to_json(Json& _json, Template const& _template)
{
	for (std::string const& parameter: _template.expect)
		requireIdentifier(parameter, "Template parameter");
	solRequire(_template.body, EthdebugException, "Pointer template without a body.");
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
		requireIdentifier(name, "Pointer template name");
		_json["pointers"][name] = pointerTemplate;
	}
}
