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
 * Yul dialects for EVM.
 */

#include <libyul/backends/evm/EVMDialect.h>

#include <libevmasm/Instruction.h>
#include <libevmasm/SemanticInformation.h>
#include <liblangutil/Exceptions.h>
#include <libsolutil/StringUtils.h>
#include <libyul/AST.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmParser.h>
#include <libyul/Exceptions.h>
#include <libyul/Object.h>
#include <libyul/Utilities.h>
#include <libyul/backends/evm/AbstractAssembly.h>

#include <range/v3/view/reverse.hpp>
#include <range/v3/view/tail.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/concat.hpp>

#include <regex>

using namespace std::string_literals;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

namespace
{

std::pair<std::string, BuiltinFunctionForEVM> createEVMFunction(
	langutil::EVMVersion _evmVersion,
	std::string const& _name,
	evmasm::Instruction _instruction
)
{
	evmasm::InstructionInfo info = evmasm::instructionInfo(_instruction, _evmVersion);
	BuiltinFunctionForEVM f;
	f.name = _name;
	f.parameters.resize(static_cast<size_t>(info.args));
	f.returns.resize(static_cast<size_t>(info.ret));
	f.sideEffects = EVMDialect::sideEffectsOfInstruction(_instruction);
	if (evmasm::SemanticInformation::terminatesControlFlow(_instruction))
	{
		f.controlFlowSideEffects.canContinue = false;
		if (evmasm::SemanticInformation::reverts(_instruction))
		{
			f.controlFlowSideEffects.canTerminate = false;
			f.controlFlowSideEffects.canRevert = true;
		}
		else
		{
			f.controlFlowSideEffects.canTerminate = true;
			f.controlFlowSideEffects.canRevert = false;
		}
	}
	f.isMSize = _instruction == evmasm::Instruction::MSIZE;
	f.literalArguments.clear();
	f.instruction = _instruction;
	f.generateCode = [_instruction](
		FunctionCall const&,
		AbstractAssembly& _assembly,
		BuiltinContext&
	) {
		_assembly.appendInstruction(_instruction);
	};

	auto const name = f.name;
	return {name, std::move(f)};
}

std::pair<std::string, BuiltinFunctionForEVM> createFunction(
	std::string const& _name,
	size_t _params,
	size_t _returns,
	SideEffects _sideEffects,
	std::vector<std::optional<LiteralKind>> _literalArguments,
	std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&)> _generateCode
)
{
	yulAssert(_literalArguments.size() == _params || _literalArguments.empty(), "");

	BuiltinFunctionForEVM f;
	f.name = _name;
	f.parameters.resize(_params);
	f.returns.resize(_returns);
	f.sideEffects = std::move(_sideEffects);
	f.literalArguments = std::move(_literalArguments);
	f.isMSize = false;
	f.instruction = {};
	f.generateCode = std::move(_generateCode);
	return {f.name, f};
}

EVMDialect::ReservedIdentifiers createReservedIdentifiers(langutil::EVMVersion _evmVersion)
{
	// TODO remove this in 0.9.0. We allow creating functions or identifiers in Yul with the name
	// basefee for VMs before london.
	auto baseFeeException = [&](evmasm::Instruction _instr) -> bool
	{
		return _instr == evmasm::Instruction::BASEFEE && _evmVersion < langutil::EVMVersion::london();
	};

	// TODO remove this in 0.9.0. We allow creating functions or identifiers in Yul with the name
	// blobbasefee for VMs before cancun.
	auto blobBaseFeeException = [&](evmasm::Instruction _instr) -> bool
	{
		return _instr == evmasm::Instruction::BLOBBASEFEE && _evmVersion < langutil::EVMVersion::cancun();
	};

	// TODO remove this in 0.9.0. We allow creating functions or identifiers in Yul with the name
	// mcopy for VMs before london.
	auto mcopyException = [&](evmasm::Instruction _instr) -> bool
	{
		return _instr == evmasm::Instruction::MCOPY && _evmVersion < langutil::EVMVersion::cancun();
	};

	// TODO remove this in 0.9.0. We allow creating functions or identifiers in Yul with the name
	// prevrandao for VMs before paris.
	auto prevRandaoException = [&](std::string const& _instrName) -> bool
	{
		// Using string comparison as the opcode is the same as for "difficulty"
		return _instrName == "prevrandao" && _evmVersion < langutil::EVMVersion::paris();
	};

	// TODO remove this in 0.9.0. We allow creating functions or identifiers in Yul with the name
	// blobhash for VMs before cancun.
	auto blobHashException = [&](evmasm::Instruction _instr) -> bool
	{
		return _instr == evmasm::Instruction::BLOBHASH && _evmVersion < langutil::EVMVersion::cancun();
	};
	// TODO remove this in 0.9.0. We allow creating functions or identifiers in Yul with the names
	// tstore or tload for VMs before cancun.
	auto transientStorageException = [&](evmasm::Instruction _instr) -> bool
	{
		return
			_evmVersion < langutil::EVMVersion::cancun() &&
			(_instr == evmasm::Instruction::TSTORE || _instr == evmasm::Instruction::TLOAD);
	};

	EVMDialect::ReservedIdentifiers reserved;
	for (auto const& instr: evmasm::c_instructions)
	{
		std::string name = toLower(instr.first);
		if (
			!baseFeeException(instr.second) &&
			!prevRandaoException(name) &&
			!blobHashException(instr.second) &&
			!blobBaseFeeException(instr.second) &&
			!mcopyException(instr.second) &&
			!transientStorageException(instr.second)
		)
			reserved.emplace(name);
	}
	reserved += std::vector<std::string>{
		"linkersymbol",
		"datasize",
		"dataoffset",
		"datacopy",
		"setimmutable",
		"loadimmutable"
	};
	return reserved;
}

EVMDialect::BuiltinsMap createBuiltins(langutil::EVMVersion _evmVersion, bool _objectAccess)
{

	// Exclude prevrandao as builtin for VMs before paris and difficulty for VMs after paris.
	auto prevRandaoException = [&](std::string const& _instrName) -> bool
	{
		return (_instrName == "prevrandao" && _evmVersion < langutil::EVMVersion::paris()) || (_instrName == "difficulty" && _evmVersion >= langutil::EVMVersion::paris());
	};

	EVMDialect::BuiltinsMap builtins;
	for (auto const& instr: evmasm::c_instructions)
	{
		std::string name = toLower(instr.first);
		auto const opcode = instr.second;

		if (
			!evmasm::isDupInstruction(opcode) &&
			!evmasm::isSwapInstruction(opcode) &&
			!evmasm::isPushInstruction(opcode) &&
			opcode != evmasm::Instruction::JUMP &&
			opcode != evmasm::Instruction::JUMPI &&
			opcode != evmasm::Instruction::JUMPDEST &&
			_evmVersion.hasOpcode(opcode) &&
			!prevRandaoException(name)
		)
			builtins.emplace(createEVMFunction(_evmVersion, name, opcode));
	}

	if (_objectAccess)
	{
		builtins.emplace(createFunction("linkersymbol", 1, 1, SideEffects{}, {LiteralKind::String}, [](
			FunctionCall const& _call,
			AbstractAssembly& _assembly,
			BuiltinContext&
		) {
			yulAssert(_call.arguments.size() == 1, "");
			Expression const& arg = _call.arguments.front();
			_assembly.appendLinkerSymbol(formatLiteral(std::get<Literal>(arg)));
		}));

		builtins.emplace(createFunction(
			"memoryguard",
			1,
			1,
			SideEffects{},
			{LiteralKind::Number},
			[](
				FunctionCall const& _call,
				AbstractAssembly& _assembly,
				BuiltinContext&
			) {
				yulAssert(_call.arguments.size() == 1, "");
				Literal const* literal = std::get_if<Literal>(&_call.arguments.front());
				yulAssert(literal, "");
				_assembly.appendConstant(literal->value.value());
			})
		);

		builtins.emplace(createFunction("datasize", 1, 1, SideEffects{}, {LiteralKind::String}, [](
			FunctionCall const& _call,
			AbstractAssembly& _assembly,
			BuiltinContext& _context
		) {
			yulAssert(_context.currentObject, "No object available.");
			yulAssert(_call.arguments.size() == 1, "");
			Expression const& arg = _call.arguments.front();
			auto const dataName = formatLiteral(std::get<Literal>(arg));
			if (_context.currentObject->name == dataName)
				_assembly.appendAssemblySize();
			else
			{
			std::vector<size_t> subIdPath =
					_context.subIDs.count(dataName) == 0 ?
						_context.currentObject->pathToSubObject(dataName) :
						std::vector<size_t>{_context.subIDs.at(dataName)};
				yulAssert(!subIdPath.empty(), "Could not find assembly object <" + dataName + ">.");
				_assembly.appendDataSize(subIdPath);
			}
		}));
		builtins.emplace(createFunction("dataoffset", 1, 1, SideEffects{}, {LiteralKind::String}, [](
			FunctionCall const& _call,
			AbstractAssembly& _assembly,
			BuiltinContext& _context
		) {
			yulAssert(_context.currentObject, "No object available.");
			yulAssert(_call.arguments.size() == 1, "");
			Expression const& arg = _call.arguments.front();
			auto const dataName = formatLiteral(std::get<Literal>(arg));
			if (_context.currentObject->name == dataName)
				_assembly.appendConstant(0);
			else
			{
			std::vector<size_t> subIdPath =
					_context.subIDs.count(dataName) == 0 ?
						_context.currentObject->pathToSubObject(dataName) :
						std::vector<size_t>{_context.subIDs.at(dataName)};
				yulAssert(!subIdPath.empty(), "Could not find assembly object <" + dataName + ">.");
				_assembly.appendDataOffset(subIdPath);
			}
		}));
		builtins.emplace(createFunction(
			"datacopy",
			3,
			0,
			SideEffects{
				false,               // movable
				true,                // movableApartFromEffects
				false,               // canBeRemoved
				false,               // canBeRemovedIfNotMSize
				true,                // cannotLoop
				SideEffects::None,   // otherState
				SideEffects::None,   // storage
				SideEffects::Write,  // memory
				SideEffects::None    // transientStorage
			},
			{},
			[](
				FunctionCall const&,
				AbstractAssembly& _assembly,
				BuiltinContext&
			) {
				_assembly.appendInstruction(evmasm::Instruction::CODECOPY);
			}
		));
		builtins.emplace(createFunction(
			"setimmutable",
			3,
			0,
			SideEffects{
				false,               // movable
				false,               // movableApartFromEffects
				false,               // canBeRemoved
				false,               // canBeRemovedIfNotMSize
				true,                // cannotLoop
				SideEffects::None,   // otherState
				SideEffects::None,   // storage
				SideEffects::Write,  // memory
				SideEffects::None    // transientStorage
			},
			{std::nullopt, LiteralKind::String, std::nullopt},
			[](
				FunctionCall const& _call,
				AbstractAssembly& _assembly,
				BuiltinContext&
			) {
				yulAssert(_call.arguments.size() == 3, "");
				auto const identifier = (formatLiteral(std::get<Literal>(_call.arguments[1])));
				_assembly.appendImmutableAssignment(identifier);
			}
		));
		builtins.emplace(createFunction(
			"loadimmutable",
			1,
			1,
			SideEffects{},
			{LiteralKind::String},
			[](
				FunctionCall const& _call,
				AbstractAssembly& _assembly,
				BuiltinContext&
			) {
				yulAssert(_call.arguments.size() == 1, "");
				_assembly.appendImmutable(formatLiteral(std::get<Literal>(_call.arguments.front())));
			}
		));
	}
	return builtins;
}

std::regex const& verbatimPattern()
{
	std::regex static const pattern{"verbatim_([1-9]?[0-9])i_([1-9]?[0-9])o"};
	return pattern;
}

}


EVMDialect::EVMDialect(langutil::EVMVersion _evmVersion, bool _objectAccess):
	m_objectAccess(_objectAccess),
	m_evmVersion(_evmVersion),
	m_functions(createBuiltins(_evmVersion, _objectAccess)),
	m_reserved(createReservedIdentifiers(_evmVersion))
{
}

BuiltinFunctionForEVM const* EVMDialect::builtin(std::string_view const _name) const
{
	if (m_objectAccess)
	{
		std::smatch match;
		auto const input = std::string(_name);
		if (std::regex_match(input, match, verbatimPattern()))
			return verbatimFunction(stoul(match[1]), stoul(match[2]));
	}
	auto it = m_functions.find(_name);
	if (it != m_functions.end())
		return &it->second;
	else
		return nullptr;
}

BuiltinFunctionForEVM const* EVMDialect::builtinNoVerbatim(std::string_view const _name) const
{
	auto const it = m_functions.find(_name);
	if (it != m_functions.end())
		return &it->second;
	else
		return nullptr;
}

bool EVMDialect::reservedIdentifier(std::string_view const _name) const
{
	if (m_objectAccess)
		if (_name.substr(0, "verbatim"s.size()) == "verbatim")
			return true;
	return m_reserved.count(_name) != 0;
}

EVMDialect const& EVMDialect::strictAssemblyForEVM(langutil::EVMVersion _version)
{
	static std::map<langutil::EVMVersion, std::unique_ptr<EVMDialect const>> dialects;
	if (!dialects[_version])
		dialects[_version] = std::make_unique<EVMDialect>(_version, false);
	return *dialects[_version];
}

EVMDialect const& EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion _version)
{
	static std::map<langutil::EVMVersion, std::unique_ptr<EVMDialect const>> dialects;
	if (!dialects[_version])
		dialects[_version] = std::make_unique<EVMDialect>(_version, true);
	return *dialects[_version];
}

SideEffects EVMDialect::sideEffectsOfInstruction(evmasm::Instruction _instruction)
{
	auto translate = [](evmasm::SemanticInformation::Effect _e) -> SideEffects::Effect
	{
		return static_cast<SideEffects::Effect>(_e);
	};

	return SideEffects{
		evmasm::SemanticInformation::movable(_instruction),
		evmasm::SemanticInformation::movableApartFromEffects(_instruction),
		evmasm::SemanticInformation::canBeRemoved(_instruction),
		evmasm::SemanticInformation::canBeRemovedIfNoMSize(_instruction),
		true, // cannotLoop
		translate(evmasm::SemanticInformation::otherState(_instruction)),
		translate(evmasm::SemanticInformation::storage(_instruction)),
		translate(evmasm::SemanticInformation::memory(_instruction)),
		translate(evmasm::SemanticInformation::transientStorage(_instruction)),
	};
}

BuiltinFunctionForEVM const* EVMDialect::verbatimFunction(size_t _arguments, size_t _returnVariables) const
{
	std::pair<size_t, size_t> key{_arguments, _returnVariables};
	auto [it, emplaced] = m_verbatimFunctions.try_emplace(std::make_pair(_arguments, _returnVariables));
	if (emplaced)
	{
		auto const name = "verbatim_" + std::to_string(_arguments) + "i_" + std::to_string(_returnVariables) + "o";
		BuiltinFunctionForEVM builtinFunction = createFunction(
			name,
			1 + _arguments,
			_returnVariables,
			SideEffects::worst(),
			std::vector<std::optional<LiteralKind>>{LiteralKind::String} + std::vector<std::optional<LiteralKind>>(_arguments),
			[=](
				FunctionCall const& _call,
				AbstractAssembly& _assembly,
				BuiltinContext&
			) {
				yulAssert(_call.arguments.size() == (1 + _arguments), "");
				Expression const& bytecode = _call.arguments.front();

				_assembly.appendVerbatim(
					asBytes(formatLiteral(std::get<Literal>(bytecode))),
					_arguments,
					_returnVariables
				);
			}
		).second;
		builtinFunction.isMSize = true;
		std::fill(builtinFunction.parameters.begin(), builtinFunction.parameters.end(), defaultType);
		std::fill(builtinFunction.returns.begin(), builtinFunction.returns.end(), defaultType);
		it->second = std::make_shared<BuiltinFunctionForEVM const>(std::move(builtinFunction));
	}
	return it->second.get();
}

std::set<std::string> EVMDialect::builtinNames() const {
	std::set<std::string> builtins{"verbatim"};
	for (auto const& [key, _]: m_functions)
		builtins.insert(key);
	return builtins;
}

EVMDialectTyped::EVMDialectTyped(langutil::EVMVersion _evmVersion, bool _objectAccess):
	EVMDialect(_evmVersion, _objectAccess)
{
	defaultType = "u256";
	boolType = "bool";
	types = {defaultType, boolType};
	not_ = "not";
	popbool_ = "popbool";

	// Set all types to ``defaultType``
	for (auto& fun: m_functions)
	{
		for (auto& p: fun.second.parameters)
			p = defaultType;
		for (auto& r: fun.second.returns)
			r = defaultType;
	}

	m_functions["lt"].returns = {boolType};
	m_functions["gt"].returns = {boolType};
	m_functions["slt"].returns = {boolType};
	m_functions["sgt"].returns = {boolType};
	m_functions["eq"].returns = {boolType};

	// "not" and "bitnot" replace "iszero" and "not"
	m_functions["bitnot"] = m_functions[not_];
	m_functions["bitnot"].name = "bitnot";
	m_functions[not_] = m_functions["iszero"];
	m_functions[not_].name = not_;
	m_functions[not_].returns = {boolType};
	m_functions[not_].parameters = {boolType};
	m_functions.erase("iszero");

	m_functions["bitand"] = m_functions["and"];
	m_functions["bitand"].name = "bitand";
	m_functions["bitor"] = m_functions["or"];
	m_functions["bitor"].name = "bitor";
	m_functions["bitxor"] = m_functions["xor"];
	m_functions["bitxor"].name = "bitxor";
	m_functions["and"].parameters = {boolType, boolType};
	m_functions["and"].returns = {boolType};
	m_functions["or"].parameters = {boolType, boolType};
	m_functions["or"].returns = {boolType};
	m_functions["xor"].parameters = {boolType, boolType};
	m_functions["xor"].returns = {boolType};
	m_functions["popbool"] = m_functions["pop"];
	m_functions["popbool"].name = "popbool";
	m_functions["popbool"].parameters = {boolType};
	m_functions.insert(createFunction("bool_to_u256", 1, 1, {}, {}, [](
		FunctionCall const&,
		AbstractAssembly&,
		BuiltinContext&
	) {}));
	m_functions["bool_to_u256"].parameters = {boolType};
	m_functions["bool_to_u256"].returns = {defaultType};
	m_functions.insert(createFunction("u256_to_bool", 1, 1, {}, {}, [](
		FunctionCall const&,
		AbstractAssembly& _assembly,
		BuiltinContext&
	) {
		// TODO this should use a Panic.
		// A value larger than 1 causes an invalid instruction.
		_assembly.appendConstant(2);
		_assembly.appendInstruction(evmasm::Instruction::DUP2);
		_assembly.appendInstruction(evmasm::Instruction::LT);
		AbstractAssembly::LabelID inRange = _assembly.newLabelId();
		_assembly.appendJumpToIf(inRange);
		_assembly.appendInstruction(evmasm::Instruction::INVALID);
		_assembly.appendLabel(inRange);
	}));
	m_functions["u256_to_bool"].parameters = {defaultType};
	m_functions["u256_to_bool"].returns = {boolType};
}

BuiltinFunctionForEVM const* EVMDialectTyped::discardFunction(std::string_view const _type) const
{
	if (_type == boolType)
		return builtinNoVerbatim(popbool_);
	else
	{
		yulAssert(_type == defaultType, "");
		return builtinNoVerbatim("pop");
	}
}

BuiltinFunctionForEVM const* EVMDialectTyped::equalityFunction(std::string_view _type) const
{
	if (_type == boolType)
		return nullptr;
	else
	{
		yulAssert(_type == defaultType, "");
		return builtinNoVerbatim("eq");
	}
}

EVMDialectTyped const& EVMDialectTyped::instance(langutil::EVMVersion _version)
{
	static std::map<langutil::EVMVersion, std::unique_ptr<EVMDialectTyped const>> dialects;
	if (!dialects[_version])
		dialects[_version] = std::make_unique<EVMDialectTyped>(_version, true);
	return *dialects[_version];
}
