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
 * Component that transforms internal Wasm representation to binary.
 */

#include <libyul/backends/wasm/BinaryTransform.h>

#include <libyul/Exceptions.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/Visitor.h>
#include <libsolutil/LEB128.h>

#include <range/v3/view/map.hpp>
#include <range/v3/view/reverse.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::yul::wasm;
using namespace solidity::util;

namespace
{

bytes toBytes(uint8_t _b)
{
	return bytes(1, _b);
}

enum class LimitsKind: uint8_t
{
	Min = 0x00,
	MinMax = 0x01,
};

enum class Mutability: uint8_t
{
	Const = 0x00,
	Var = 0x01,
};

enum class Section: uint8_t
{
	CUSTOM = 0x00,
	TYPE = 0x01,
	IMPORT = 0x02,
	FUNCTION = 0x03,
	MEMORY = 0x05,
	GLOBAL = 0x06,
	EXPORT = 0x07,
	CODE = 0x0a
};

bytes toBytes(Section _s)
{
	return toBytes(uint8_t(_s));
}

enum class ValueType: uint8_t
{
	Void = 0x40,
	Function = 0x60,
	I64 = 0x7e,
	I32 = 0x7f
};

bytes toBytes(ValueType _vt)
{
	return toBytes(uint8_t(_vt));
}

ValueType toValueType(wasm::Type _type)
{
	if (_type == wasm::Type::i32)
		return ValueType::I32;
	else if (_type == wasm::Type::i64)
		return ValueType::I64;
	else
		yulAssert(false, "Invalid wasm variable type");
}

enum class Export: uint8_t
{
	Function = 0x0,
	Memory = 0x2
};

bytes toBytes(Export _export)
{
	return toBytes(uint8_t(_export));
}

// NOTE: This is a subset of WebAssembly opcodes.
//       Those available as a builtin are listed further down.
enum class Opcode: uint8_t
{
	Block = 0x02,
	Loop = 0x03,
	If = 0x04,
	Else = 0x05,
	End = 0x0b,
	Br = 0x0c,
	BrIf = 0x0d,
	BrTable = 0x0e, // Not used yet.
	Return = 0x0f,
	Call = 0x10,
	CallIndirect = 0x11, // Not used yet.
	LocalGet = 0x20,
	LocalSet = 0x21,
	LocalTee = 0x22, // Not used yet.
	GlobalGet = 0x23,
	GlobalSet = 0x24,
	I32Const = 0x41,
	I64Const = 0x42,
};

bytes toBytes(Opcode _o)
{
	return toBytes(uint8_t(_o));
}

Opcode constOpcodeFor(ValueType _type)
{
	if (_type == ValueType::I32)
		return Opcode::I32Const;
	else if (_type == ValueType::I64)
		return Opcode::I64Const;
	else
		yulAssert(false, "Values of this type cannot be used with const opcode");
}

static map<string, uint8_t> const builtins = {
	{"unreachable", 0x00},
	{"nop", 0x01},
	{"i32.drop", 0x1a},
	{"i64.drop", 0x1a},
	{"i32.select", 0x1b},
	{"i64.select", 0x1b},
	{"i32.load", 0x28},
	{"i64.load", 0x29},
	{"i32.load8_s", 0x2c},
	{"i32.load8_u", 0x2d},
	{"i32.load16_s", 0x2e},
	{"i32.load16_u", 0x2f},
	{"i64.load8_s", 0x30},
	{"i64.load8_u", 0x31},
	{"i64.load16_s", 0x32},
	{"i64.load16_u", 0x33},
	{"i64.load32_s", 0x34},
	{"i64.load32_u", 0x35},
	{"i32.store", 0x36},
	{"i64.store", 0x37},
	{"i32.store8", 0x3a},
	{"i32.store16", 0x3b},
	{"i64.store8", 0x3c},
	{"i64.store16", 0x3d},
	{"i64.store32", 0x3e},
	{"memory.size", 0x3f},
	{"memory.grow", 0x40},
	{"i32.eqz", 0x45},
	{"i32.eq", 0x46},
	{"i32.ne", 0x47},
	{"i32.lt_s", 0x48},
	{"i32.lt_u", 0x49},
	{"i32.gt_s", 0x4a},
	{"i32.gt_u", 0x4b},
	{"i32.le_s", 0x4c},
	{"i32.le_u", 0x4d},
	{"i32.ge_s", 0x4e},
	{"i32.ge_u", 0x4f},
	{"i64.eqz", 0x50},
	{"i64.eq", 0x51},
	{"i64.ne", 0x52},
	{"i64.lt_s", 0x53},
	{"i64.lt_u", 0x54},
	{"i64.gt_s", 0x55},
	{"i64.gt_u", 0x56},
	{"i64.le_s", 0x57},
	{"i64.le_u", 0x58},
	{"i64.ge_s", 0x59},
	{"i64.ge_u", 0x5a},
	{"i32.clz", 0x67},
	{"i32.ctz", 0x68},
	{"i32.popcnt", 0x69},
	{"i32.add", 0x6a},
	{"i32.sub", 0x6b},
	{"i32.mul", 0x6c},
	{"i32.div_s", 0x6d},
	{"i32.div_u", 0x6e},
	{"i32.rem_s", 0x6f},
	{"i32.rem_u", 0x70},
	{"i32.and", 0x71},
	{"i32.or", 0x72},
	{"i32.xor", 0x73},
	{"i32.shl", 0x74},
	{"i32.shr_s", 0x75},
	{"i32.shr_u", 0x76},
	{"i32.rotl", 0x77},
	{"i32.rotr", 0x78},
	{"i64.clz", 0x79},
	{"i64.ctz", 0x7a},
	{"i64.popcnt", 0x7b},
	{"i64.add", 0x7c},
	{"i64.sub", 0x7d},
	{"i64.mul", 0x7e},
	{"i64.div_s", 0x7f},
	{"i64.div_u", 0x80},
	{"i64.rem_s", 0x81},
	{"i64.rem_u", 0x82},
	{"i64.and", 0x83},
	{"i64.or", 0x84},
	{"i64.xor", 0x85},
	{"i64.shl", 0x86},
	{"i64.shr_s", 0x87},
	{"i64.shr_u", 0x88},
	{"i64.rotl", 0x89},
	{"i64.rotr", 0x8a},
	{"i32.wrap_i64", 0xa7},
	{"i64.extend_i32_s", 0xac},
	{"i64.extend_i32_u", 0xad},
};

bytes prefixSize(bytes _data)
{
	size_t size = _data.size();
	return lebEncode(size) + std::move(_data);
}

bytes makeSection(Section _section, bytes _data)
{
	return toBytes(_section) + prefixSize(std::move(_data));
}

/// This is a kind of run-length-encoding of local types.
vector<pair<size_t, ValueType>> groupLocalVariables(vector<VariableDeclaration> _localVariables)
{
	vector<pair<size_t, ValueType>> localEntries;

	size_t entrySize = 0;
	ValueType entryType = ValueType::I32; // Any type would work here
	for (VariableDeclaration const& localVariable: _localVariables)
	{
		ValueType variableType = toValueType(localVariable.type);

		if (variableType != entryType)
		{
			if (entrySize > 0)
				localEntries.emplace_back(entrySize, entryType);

			entryType = variableType;
			entrySize = 0;
		}

		++entrySize;
	}
	if (entrySize > 0)
		localEntries.emplace_back(entrySize, entryType);

	return localEntries;
}

}

bytes BinaryTransform::run(Module const& _module)
{
	map<Type, vector<string>> const types = typeToFunctionMap(_module.imports, _module.functions);

	map<string, size_t> const globalIDs = enumerateGlobals(_module);
	map<string, size_t> const functionIDs = enumerateFunctions(_module);
	map<string, size_t> const functionTypes = enumerateFunctionTypes(types);

	yulAssert(globalIDs.size() == _module.globals.size(), "");
	yulAssert(functionIDs.size() == _module.imports.size() + _module.functions.size(), "");
	yulAssert(functionTypes.size() == functionIDs.size(), "");
	yulAssert(functionTypes.size() >= types.size(), "");

	bytes ret{0, 'a', 's', 'm'};
	// version
	ret += bytes{1, 0, 0, 0};
	ret += typeSection(types);
	ret += importSection(_module.imports, functionTypes);
	ret += functionSection(_module.functions, functionTypes);
	ret += memorySection();
	ret += globalSection(_module.globals);
	ret += exportSection(functionIDs);

	map<string, pair<size_t, size_t>> subModulePosAndSize;
	for (auto const& [name, module]: _module.subModules)
	{
		// TODO should we prefix and / or shorten the name?
		bytes data = BinaryTransform::run(module);
		size_t const length = data.size();
		ret += customSection(name, std::move(data));
		// Skip all the previous sections and the size field of this current custom section.
		size_t const offset = ret.size() - length;
		subModulePosAndSize[name] = {offset, length};
	}
	for (auto const& [name, data]: _module.customSections)
	{
		size_t const length = data.size();
		ret += customSection(name, data);
		// Skip all the previous sections and the size field of this current custom section.
		size_t const offset = ret.size() - length;
		subModulePosAndSize[name] = {offset, length};
	}

	BinaryTransform bt(
		std::move(globalIDs),
		std::move(functionIDs),
		std::move(functionTypes),
		std::move(subModulePosAndSize)
	);

	ret += bt.codeSection(_module.functions);
	return ret;
}

bytes BinaryTransform::operator()(Literal const& _literal)
{
	return std::visit(GenericVisitor{
		[&](uint32_t _value) -> bytes { return toBytes(Opcode::I32Const) + lebEncodeSigned(static_cast<int32_t>(_value)); },
		[&](uint64_t _value) -> bytes { return toBytes(Opcode::I64Const) + lebEncodeSigned(static_cast<int64_t>(_value)); },
	}, _literal.value);
}

bytes BinaryTransform::operator()(StringLiteral const&)
{
	// StringLiteral is a special AST element used for certain builtins.
	// It is not mapped to actual WebAssembly, and should be processed in visit(BuiltinCall).
	yulAssert(false, "");
}

bytes BinaryTransform::operator()(LocalVariable const& _variable)
{
	return toBytes(Opcode::LocalGet) + lebEncode(m_locals.at(_variable.name));
}

bytes BinaryTransform::operator()(GlobalVariable const& _variable)
{
	return toBytes(Opcode::GlobalGet) + lebEncode(m_globalIDs.at(_variable.name));
}

bytes BinaryTransform::operator()(BuiltinCall const& _call)
{
	// We need to avoid visiting the arguments of `dataoffset` and `datasize` because
	// they are references to object names that should not end up in the code.
	if (_call.functionName == "dataoffset")
	{
		string name = get<StringLiteral>(_call.arguments.at(0)).value;
		// TODO: support the case where name refers to the current object
		yulAssert(m_subModulePosAndSize.count(name), "");
		return toBytes(Opcode::I64Const) + lebEncodeSigned(static_cast<int64_t>(m_subModulePosAndSize.at(name).first));
	}
	else if (_call.functionName == "datasize")
	{
		string name = get<StringLiteral>(_call.arguments.at(0)).value;
		// TODO: support the case where name refers to the current object
		yulAssert(m_subModulePosAndSize.count(name), "");
		return toBytes(Opcode::I64Const) + lebEncodeSigned(static_cast<int64_t>(m_subModulePosAndSize.at(name).second));
	}

	yulAssert(builtins.count(_call.functionName), "Builtin " + _call.functionName + " not found");
	// NOTE: the dialect ensures we have the right amount of arguments
	bytes args = visit(_call.arguments);
	bytes ret = std::move(args) + toBytes(builtins.at(_call.functionName));
	if (
		_call.functionName.find(".load") != string::npos ||
		_call.functionName.find(".store") != string::npos
	)
		// Alignment hint and offset. Interpreters ignore the alignment. JITs/AOTs can take it
		// into account to generate more efficient code but if the hint is invalid it could
		// actually be more expensive. It's best to hint at 1-byte alignment if we don't plan
		// to control the memory layout accordingly.
		ret += bytes{{0, 0}}; // 2^0 == 1-byte alignment

	return ret;
}

bytes BinaryTransform::operator()(FunctionCall const& _call)
{
	return visit(_call.arguments) + toBytes(Opcode::Call) + lebEncode(m_functionIDs.at(_call.functionName));
}

bytes BinaryTransform::operator()(LocalAssignment const& _assignment)
{
	return
		std::visit(*this, *_assignment.value) +
		toBytes(Opcode::LocalSet) +
		lebEncode(m_locals.at(_assignment.variableName));
}

bytes BinaryTransform::operator()(GlobalAssignment const& _assignment)
{
	return
		std::visit(*this, *_assignment.value) +
		toBytes(Opcode::GlobalSet) +
		lebEncode(m_globalIDs.at(_assignment.variableName));
}

bytes BinaryTransform::operator()(If const& _if)
{
	bytes result =
		std::visit(*this, *_if.condition) +
		toBytes(Opcode::If) +
		toBytes(ValueType::Void);

	m_labels.emplace_back();

	result += visit(_if.statements);
	if (_if.elseStatements)
		result += toBytes(Opcode::Else) + visit(*_if.elseStatements);

	m_labels.pop_back();

	result += toBytes(Opcode::End);
	return result;
}

bytes BinaryTransform::operator()(Loop const& _loop)
{
	bytes result = toBytes(Opcode::Loop) + toBytes(ValueType::Void);

	m_labels.emplace_back(_loop.labelName);
	result += visit(_loop.statements);
	m_labels.pop_back();

	result += toBytes(Opcode::End);
	return result;
}

bytes BinaryTransform::operator()(Branch const& _branch)
{
	return toBytes(Opcode::Br) + encodeLabelIdx(_branch.label.name);
}

bytes BinaryTransform::operator()(BranchIf const& _branchIf)
{
	bytes result = std::visit(*this, *_branchIf.condition);
	result += toBytes(Opcode::BrIf) + encodeLabelIdx(_branchIf.label.name);
	return result;
}

bytes BinaryTransform::operator()(Return const&)
{
	// Note that this does not work if the function returns a value.
	return toBytes(Opcode::Return);
}

bytes BinaryTransform::operator()(Block const& _block)
{
	m_labels.emplace_back(_block.labelName);
	bytes result =
		toBytes(Opcode::Block) +
		toBytes(ValueType::Void) +
		visit(_block.statements) +
		toBytes(Opcode::End);
	m_labels.pop_back();
	return result;
}

bytes BinaryTransform::operator()(FunctionDefinition const& _function)
{
	bytes ret;

	vector<pair<size_t, ValueType>> localEntries = groupLocalVariables(_function.locals);
	ret += lebEncode(localEntries.size());
	for (pair<size_t, ValueType> const& entry: localEntries)
	{
		ret += lebEncode(entry.first);
		ret += toBytes(entry.second);
	}

	m_locals.clear();
	size_t varIdx = 0;
	for (size_t i = 0; i < _function.parameters.size(); ++i)
		m_locals[_function.parameters[i].name] = varIdx++;
	for (size_t i = 0; i < _function.locals.size(); ++i)
		m_locals[_function.locals[i].variableName] = varIdx++;

	yulAssert(m_labels.empty(), "Stray labels.");

	ret += visit(_function.body);
	ret += toBytes(Opcode::End);

	yulAssert(m_labels.empty(), "Stray labels.");

	return prefixSize(std::move(ret));
}

BinaryTransform::Type BinaryTransform::typeOf(FunctionImport const& _import)
{
	return {
		encodeTypes(_import.paramTypes),
		encodeTypes(_import.returnType ? vector<wasm::Type>(1, *_import.returnType) : vector<wasm::Type>())
	};
}

BinaryTransform::Type BinaryTransform::typeOf(FunctionDefinition const& _funDef)
{
	return {
		encodeTypes(_funDef.parameters),
		encodeTypes(_funDef.returnType ? vector<wasm::Type>(1, *_funDef.returnType) : vector<wasm::Type>())
	};
}

uint8_t BinaryTransform::encodeType(wasm::Type _type)
{
	return uint8_t(toValueType(_type));
}

vector<uint8_t> BinaryTransform::encodeTypes(vector<wasm::Type> const& _types)
{
	vector<uint8_t> result;
	for (wasm::Type t: _types)
		result.emplace_back(encodeType(t));
	return result;
}

vector<uint8_t> BinaryTransform::encodeTypes(wasm::TypedNameList const& _typedNameList)
{
	vector<uint8_t> result;
	for (TypedName const& typedName: _typedNameList)
		result.emplace_back(encodeType(typedName.type));
	return result;
}

map<BinaryTransform::Type, vector<string>> BinaryTransform::typeToFunctionMap(
	vector<wasm::FunctionImport> const& _imports,
	vector<wasm::FunctionDefinition> const& _functions
)
{
	map<Type, vector<string>> types;
	for (auto const& import: _imports)
		types[typeOf(import)].emplace_back(import.internalName);
	for (auto const& fun: _functions)
		types[typeOf(fun)].emplace_back(fun.name);

	return types;
}

map<string, size_t> BinaryTransform::enumerateGlobals(Module const& _module)
{
	map<string, size_t> globals;
	for (size_t i = 0; i < _module.globals.size(); ++i)
		globals[_module.globals[i].variableName] = i;

	return globals;
}

map<string, size_t> BinaryTransform::enumerateFunctions(Module const& _module)
{
	map<string, size_t> functions;
	size_t funID = 0;
	for (FunctionImport const& fun: _module.imports)
		functions[fun.internalName] = funID++;
	for (FunctionDefinition const& fun: _module.functions)
		functions[fun.name] = funID++;

	return functions;
}

map<string, size_t> BinaryTransform::enumerateFunctionTypes(map<Type, vector<string>> const& _typeToFunctionMap)
{
	map<string, size_t> functionTypes;
	size_t typeID = 0;
	for (vector<string> const& funNames: _typeToFunctionMap | ranges::views::values)
	{
		for (string const& name: funNames)
			functionTypes[name] = typeID;
		++typeID;
	}

	return functionTypes;
}

bytes BinaryTransform::typeSection(map<BinaryTransform::Type, vector<string>> const& _typeToFunctionMap)
{
	bytes result;
	size_t index = 0;
	for (Type const& type: _typeToFunctionMap | ranges::views::keys)
	{
		result += toBytes(ValueType::Function);
		result += lebEncode(type.first.size()) + type.first;
		result += lebEncode(type.second.size()) + type.second;

		index++;
	}

	return makeSection(Section::TYPE, lebEncode(index) + std::move(result));
}

bytes BinaryTransform::importSection(
	vector<FunctionImport> const& _imports,
	map<string, size_t> const& _functionTypes
)
{
	bytes result = lebEncode(_imports.size());
	for (FunctionImport const& import: _imports)
	{
		uint8_t importKind = 0; // function
		result +=
			encodeName(import.module) +
			encodeName(import.externalName) +
			toBytes(importKind) +
			lebEncode(_functionTypes.at(import.internalName));
	}
	return makeSection(Section::IMPORT, std::move(result));
}

bytes BinaryTransform::functionSection(
	vector<FunctionDefinition> const& _functions,
	map<string, size_t> const& _functionTypes
)
{
	bytes result = lebEncode(_functions.size());
	for (auto const& fun: _functions)
		result += lebEncode(_functionTypes.at(fun.name));
	return makeSection(Section::FUNCTION, std::move(result));
}

bytes BinaryTransform::memorySection()
{
	bytes result = lebEncode(1);
	result.push_back(static_cast<uint8_t>(LimitsKind::Min));
	result.push_back(1); // initial length
	return makeSection(Section::MEMORY, std::move(result));
}

bytes BinaryTransform::globalSection(vector<wasm::GlobalVariableDeclaration> const& _globals)
{
	bytes result = lebEncode(_globals.size());
	for (wasm::GlobalVariableDeclaration const& global: _globals)
	{
		ValueType globalType = toValueType(global.type);
		result +=
			toBytes(globalType) +
			lebEncode(static_cast<uint8_t>(Mutability::Var)) +
			toBytes(constOpcodeFor(globalType)) +
			lebEncodeSigned(0) +
			toBytes(Opcode::End);
	}

	return makeSection(Section::GLOBAL, std::move(result));
}

bytes BinaryTransform::exportSection(map<string, size_t> const& _functionIDs)
{
	bool hasMain = _functionIDs.count("main");
	bytes result = lebEncode(hasMain ? 2 : 1);
	result += encodeName("memory") + toBytes(Export::Memory) + lebEncode(0);
	if (hasMain)
		result += encodeName("main") + toBytes(Export::Function) + lebEncode(_functionIDs.at("main"));
	return makeSection(Section::EXPORT, std::move(result));
}

bytes BinaryTransform::customSection(string const& _name, bytes _data)
{
	bytes result = encodeName(_name) + std::move(_data);
	return makeSection(Section::CUSTOM, std::move(result));
}

bytes BinaryTransform::codeSection(vector<wasm::FunctionDefinition> const& _functions)
{
	bytes result = lebEncode(_functions.size());
	for (FunctionDefinition const& fun: _functions)
		result += (*this)(fun);
	return makeSection(Section::CODE, std::move(result));
}

bytes BinaryTransform::visit(vector<Expression> const& _expressions)
{
	bytes result;
	for (auto const& expr: _expressions)
		result += std::visit(*this, expr);
	return result;
}

bytes BinaryTransform::visitReversed(vector<Expression> const& _expressions)
{
	bytes result;
	for (auto const& expr: _expressions | ranges::views::reverse)
		result += std::visit(*this, expr);
	return result;
}

bytes BinaryTransform::encodeLabelIdx(string const& _label) const
{
	yulAssert(!_label.empty(), "Empty label.");
	size_t depth = 0;
	for (string const& label: m_labels | ranges::views::reverse)
		if (label == _label)
			return lebEncode(depth);
		else
			++depth;
	yulAssert(false, "Label not found.");
}

bytes BinaryTransform::encodeName(string const& _name)
{
	// UTF-8 is allowed here by the Wasm spec, but since all names here should stem from
	// Solidity or Yul identifiers or similar, non-ascii characters ending up here
	// is a very bad sign.
	for (char c: _name)
		yulAssert(uint8_t(c) <= 0x7f, "Non-ascii character found.");
	return lebEncode(_name.size()) + asBytes(_name);
}
