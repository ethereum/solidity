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
#include <libsolutil/StringUtils.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/Object.h>
#include <libyul/Utilities.h>
#include <libyul/backends/evm/AbstractAssembly.h>

#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/view/enumerate.hpp>

#include <regex>
#include <utility>
#include <vector>

using namespace std::string_literals;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

namespace
{

size_t constexpr toContinuousVerbatimIndex(size_t _arguments, size_t _returnVariables)
{
	return _arguments + _returnVariables * EVMDialect::verbatimMaxInputSlots;
}

std::tuple<size_t, size_t> constexpr verbatimIndexToArgsAndRets(size_t _index)
{
	size_t const numRets = _index / EVMDialect::verbatimMaxInputSlots;
	return std::make_tuple(_index - numRets * EVMDialect::verbatimMaxInputSlots, numRets);
}

BuiltinFunctionForEVM createEVMFunction(
	langutil::EVMVersion _evmVersion,
	std::string const& _name,
	evmasm::Instruction _instruction
)
{
	BuiltinFunctionForEVM f;
	evmasm::InstructionInfo info = evmasm::instructionInfo(_instruction, _evmVersion);
	f.name = _name;
	f.numParameters = static_cast<size_t>(info.args);
	f.numReturns = static_cast<size_t>(info.ret);
	f.sideEffects = EVMDialect::sideEffectsOfInstruction(_instruction);
	f.controlFlowSideEffects = ControlFlowSideEffects::fromInstruction(_instruction);
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
	return f;
}

BuiltinFunctionForEVM createFunction(
	std::string const& _name,
	size_t _params,
	size_t _returns,
	SideEffects _sideEffects,
	ControlFlowSideEffects _controlFlowSideEffects,
	std::vector<std::optional<LiteralKind>> _literalArguments,
	std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&)> _generateCode
)
{
	yulAssert(_literalArguments.size() == _params || _literalArguments.empty(), "");

	BuiltinFunctionForEVM f;
	f.name = _name;
	f.numParameters = _params;
	f.numReturns = _returns;
	f.sideEffects = _sideEffects;
	f.controlFlowSideEffects = _controlFlowSideEffects;
	f.literalArguments = std::move(_literalArguments);
	f.isMSize = false;
	f.instruction = {};
	f.generateCode = std::move(_generateCode);
	return f;
}

std::set<std::string, std::less<>> createReservedIdentifiers(langutil::EVMVersion _evmVersion, std::optional<uint8_t> _eofVersion)
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

	std::set<std::string, std::less<>> reserved;
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
		"loadimmutable",
	};

	if (_eofVersion.has_value())
		reserved += std::vector<std::string>{
			"auxdataloadn",
		};

	return reserved;
}

std::vector<std::optional<BuiltinFunctionForEVM>> createBuiltins(langutil::EVMVersion _evmVersion, std::optional<uint8_t> _eofVersion, bool _objectAccess)
{

	// Exclude prevrandao as builtin for VMs before paris and difficulty for VMs after paris.
	auto prevRandaoException = [&](std::string const& _instrName) -> bool
	{
		return (_instrName == "prevrandao" && _evmVersion < langutil::EVMVersion::paris()) || (_instrName == "difficulty" && _evmVersion >= langutil::EVMVersion::paris());
	};

	std::vector<std::optional<BuiltinFunctionForEVM>> builtins;
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
			opcode != evmasm::Instruction::DATALOADN &&
			opcode != evmasm::Instruction::EOFCREATE &&
			opcode != evmasm::Instruction::RETURNCONTRACT &&
			_evmVersion.hasOpcode(opcode, _eofVersion) &&
			!prevRandaoException(name)
		)
			builtins.emplace_back(createEVMFunction(_evmVersion, name, opcode));
		else
			builtins.emplace_back(std::nullopt);
	}

	auto const createIfObjectAccess = [_objectAccess](
		std::string const& _name,
		size_t _params,
		size_t _returns,
		SideEffects _sideEffects,
		ControlFlowSideEffects _controlFlowSideEffects,
		std::vector<std::optional<LiteralKind>> _literalArguments,
		std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&)> _generateCode
	) -> std::optional<BuiltinFunctionForEVM>
	{
		if (!_objectAccess)
			return std::nullopt;
		return createFunction(_name, _params, _returns, _sideEffects, _controlFlowSideEffects, std::move(_literalArguments), std::move(_generateCode));
	};

	builtins.emplace_back(createIfObjectAccess("linkersymbol", 1, 1, SideEffects{}, ControlFlowSideEffects{}, {LiteralKind::String}, [](
		FunctionCall const& _call,
		AbstractAssembly& _assembly,
		BuiltinContext&
	) {
		yulAssert(_call.arguments.size() == 1, "");
		Expression const& arg = _call.arguments.front();
		_assembly.appendLinkerSymbol(formatLiteral(std::get<Literal>(arg)));
	}));
	builtins.emplace_back(createIfObjectAccess(
		"memoryguard",
		1,
		1,
		SideEffects{},
		ControlFlowSideEffects{},
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
	if (!_eofVersion.has_value())
	{
		builtins.emplace_back(createIfObjectAccess("datasize", 1, 1, SideEffects{}, ControlFlowSideEffects{}, {LiteralKind::String}, [](
			FunctionCall const& _call,
			AbstractAssembly& _assembly,
			BuiltinContext& _context
		) {
			yulAssert(_context.currentObject, "No object available.");
			yulAssert(_call.arguments.size() == 1, "");
			Expression const& arg = _call.arguments.front();
			YulName const dataName (formatLiteral(std::get<Literal>(arg)));
			if (_context.currentObject->name == dataName.str())
				_assembly.appendAssemblySize();
			else
			{
			std::vector<size_t> subIdPath =
					_context.subIDs.count(dataName.str()) == 0 ?
						_context.currentObject->pathToSubObject(dataName.str()) :
						std::vector<size_t>{_context.subIDs.at(dataName.str())};
				yulAssert(!subIdPath.empty(), "Could not find assembly object <" + dataName.str() + ">.");
				_assembly.appendDataSize(subIdPath);
			}
		}));
		builtins.emplace_back(createIfObjectAccess("dataoffset", 1, 1, SideEffects{}, ControlFlowSideEffects{}, {LiteralKind::String}, [](
			FunctionCall const& _call,
			AbstractAssembly& _assembly,
			BuiltinContext& _context
		) {
			yulAssert(_context.currentObject, "No object available.");
			yulAssert(_call.arguments.size() == 1, "");
			Expression const& arg = _call.arguments.front();
			YulName const dataName (formatLiteral(std::get<Literal>(arg)));
			if (_context.currentObject->name == dataName.str())
				_assembly.appendConstant(0);
			else
			{
			std::vector<size_t> subIdPath =
					_context.subIDs.count(dataName.str()) == 0 ?
						_context.currentObject->pathToSubObject(dataName.str()) :
						std::vector<size_t>{_context.subIDs.at(dataName.str())};
				yulAssert(!subIdPath.empty(), "Could not find assembly object <" + dataName.str() + ">.");
				_assembly.appendDataOffset(subIdPath);
			}
		}));
		builtins.emplace_back(createIfObjectAccess(
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
			ControlFlowSideEffects::fromInstruction(evmasm::Instruction::CODECOPY),
			{},
			[](
				FunctionCall const&,
				AbstractAssembly& _assembly,
				BuiltinContext&
			) {
				_assembly.appendInstruction(evmasm::Instruction::CODECOPY);
			}
		));
		builtins.emplace_back(createIfObjectAccess(
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
			ControlFlowSideEffects{},
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
		builtins.emplace_back(createIfObjectAccess(
			"loadimmutable",
			1,
			1,
			SideEffects{},
			ControlFlowSideEffects{},
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
	else // EOF context
	{
		builtins.emplace_back(createFunction(
			"auxdataloadn",
			1,
			1,
			EVMDialect::sideEffectsOfInstruction(evmasm::Instruction::DATALOADN),
			ControlFlowSideEffects::fromInstruction(evmasm::Instruction::DATALOADN),
			{LiteralKind::Number},
			[](
				FunctionCall const& _call,
				AbstractAssembly& _assembly,
				BuiltinContext&
			) {
				yulAssert(_call.arguments.size() == 1);
				Literal const* literal = std::get_if<Literal>(&_call.arguments.front());
				yulAssert(literal, "");
				yulAssert(literal->value.value() <= std::numeric_limits<uint16_t>::max());
				_assembly.appendAuxDataLoadN(static_cast<uint16_t>(literal->value.value()));
			}
		));

		builtins.emplace_back(createFunction(
			"eofcreate",
			5,
			1,
			EVMDialect::sideEffectsOfInstruction(evmasm::Instruction::EOFCREATE),
			ControlFlowSideEffects::fromInstruction(evmasm::Instruction::EOFCREATE),
			{LiteralKind::String, std::nullopt, std::nullopt, std::nullopt, std::nullopt},
			[](
				FunctionCall const& _call,
				AbstractAssembly& _assembly,
				BuiltinContext& context
			) {
				yulAssert(_call.arguments.size() == 5);
				Literal const* literal = std::get_if<Literal>(&_call.arguments.front());
				auto const formattedLiteral = formatLiteral(*literal);
				yulAssert(!util::contains(formattedLiteral, '.'));
				auto const* containerID = valueOrNullptr(context.subIDs, formattedLiteral);
				yulAssert(containerID != nullptr);
				yulAssert(*containerID <= std::numeric_limits<AbstractAssembly::ContainerID>::max());
				_assembly.appendEOFCreate(static_cast<AbstractAssembly::ContainerID>(*containerID));
			}
			));

		if (_objectAccess)
			builtins.emplace_back(createFunction(
				"returncontract",
				3,
				0,
				EVMDialect::sideEffectsOfInstruction(evmasm::Instruction::RETURNCONTRACT),
				ControlFlowSideEffects::fromInstruction(evmasm::Instruction::RETURNCONTRACT),
				{LiteralKind::String, std::nullopt, std::nullopt},
				[](
					FunctionCall const& _call,
					AbstractAssembly& _assembly,
					BuiltinContext& context
				) {
					yulAssert(_call.arguments.size() == 3);
					Literal const* literal = std::get_if<Literal>(&_call.arguments.front());
					yulAssert(literal);
					auto const formattedLiteral = formatLiteral(*literal);
					yulAssert(!util::contains(formattedLiteral, '.'));
					auto const* containerID = valueOrNullptr(context.subIDs, formattedLiteral);
					yulAssert(containerID != nullptr);
					yulAssert(*containerID <= std::numeric_limits<AbstractAssembly::ContainerID>::max());
					_assembly.appendReturnContract(static_cast<AbstractAssembly::ContainerID>(*containerID));
				}
			));
	}
	yulAssert(
		ranges::all_of(builtins, [](std::optional<BuiltinFunctionForEVM> const& _builtinFunction){
			return !_builtinFunction || _builtinFunction->name.substr(0, "verbatim_"s.size()) != "verbatim_";
		}),
		"Builtin functions besides verbatim should not start with the verbatim_ prefix."
	);
	return builtins;
}

std::regex const& verbatimPattern()
{
	std::regex static const pattern{"([1-9]?[0-9])i_([1-9]?[0-9])o"};
	return pattern;
}

}


EVMDialect::EVMDialect(langutil::EVMVersion _evmVersion, std::optional<uint8_t> _eofVersion, bool _objectAccess):
	m_objectAccess(_objectAccess),
	m_evmVersion(_evmVersion),
	m_eofVersion(_eofVersion),
	m_functions(createBuiltins(_evmVersion, _eofVersion, _objectAccess)),
	m_reserved(createReservedIdentifiers(_evmVersion, _eofVersion))
{
	for (auto const& [index, maybeBuiltin]: m_functions | ranges::views::enumerate)
		if (maybeBuiltin)
			// ids are offset by the maximum number of verbatim functions
			m_builtinFunctionsByName[maybeBuiltin->name] = BuiltinHandle{index + verbatimIDOffset};

	m_discardFunction = findBuiltin("pop");
	m_equalityFunction = findBuiltin("eq");
	m_booleanNegationFunction = findBuiltin("iszero");
	m_memoryStoreFunction = findBuiltin("mstore");
	m_memoryLoadFunction = findBuiltin("mload");
	m_storageStoreFunction = findBuiltin("sstore");
	m_storageLoadFunction = findBuiltin("sload");
	m_hashFunction = findBuiltin("keccak256");
}

std::optional<BuiltinHandle> EVMDialect::findBuiltin(std::string_view _name) const
{
	if (m_objectAccess && _name.substr(0, "verbatim_"s.size()) == "verbatim_")
	{
		std::smatch match;
		std::string name(_name.substr("verbatim_"s.size()));
		if (regex_match(name, match, verbatimPattern()))
			return verbatimFunction(stoul(match[1]), stoul(match[2]));
	}

	if (
		auto it = m_builtinFunctionsByName.find(_name);
		it != m_builtinFunctionsByName.end()
	)
		return it->second;

	return std::nullopt;
}

BuiltinFunctionForEVM const& EVMDialect::builtin(BuiltinHandle const& _handle) const
{
	if (isVerbatimHandle(_handle))
	{
		yulAssert(_handle.id < verbatimIDOffset);
		auto const& verbatimFunctionPtr = m_verbatimFunctions[_handle.id];
		yulAssert(verbatimFunctionPtr);
		return *verbatimFunctionPtr;
	}

	yulAssert(_handle.id - verbatimIDOffset < m_functions.size());
	auto const& maybeBuiltin = m_functions[_handle.id - verbatimIDOffset];
	yulAssert(maybeBuiltin.has_value());
	return *maybeBuiltin;
}


bool EVMDialect::reservedIdentifier(std::string_view _name) const
{
	if (m_objectAccess)
		if (_name.substr(0, "verbatim"s.size()) == "verbatim")
			return true;
	return m_reserved.count(_name) != 0;
}

EVMDialect const& EVMDialect::strictAssemblyForEVM(langutil::EVMVersion _evmVersion, std::optional<uint8_t> _eofVersion)
{
	static std::map<std::pair<langutil::EVMVersion, std::optional<uint8_t>>, std::unique_ptr<EVMDialect const>> dialects;
	static YulStringRepository::ResetCallback callback{[&] { dialects.clear(); }};
	if (!dialects[{_evmVersion, _eofVersion}])
		dialects[{_evmVersion, _eofVersion}] = std::make_unique<EVMDialect>(_evmVersion, _eofVersion, false);
	return *dialects[{_evmVersion, _eofVersion}];
}

EVMDialect const& EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion _evmVersion, std::optional<uint8_t> _eofVersion)
{
	static std::map<std::pair<langutil::EVMVersion, std::optional<uint8_t>>, std::unique_ptr<EVMDialect const>> dialects;
	static YulStringRepository::ResetCallback callback{[&] { dialects.clear(); }};
	if (!dialects[{_evmVersion, _eofVersion}])
		dialects[{_evmVersion, _eofVersion}] = std::make_unique<EVMDialect>(_evmVersion, _eofVersion, true);
	return *dialects[{_evmVersion, _eofVersion}];
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

BuiltinFunctionForEVM EVMDialect::createVerbatimFunctionFromHandle(BuiltinHandle const& _handle)
{
	return std::apply(createVerbatimFunction, verbatimIndexToArgsAndRets(_handle.id));
}

BuiltinFunctionForEVM EVMDialect::createVerbatimFunction(size_t _arguments, size_t _returnVariables)
{
	BuiltinFunctionForEVM builtinFunction = createFunction(
		"verbatim_" + std::to_string(_arguments) + "i_" + std::to_string(_returnVariables) + "o",
		1 + _arguments,
		_returnVariables,
		SideEffects::worst(),
		ControlFlowSideEffects{},
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
	);
	builtinFunction.isMSize = true;
	return builtinFunction;
}

BuiltinHandle EVMDialect::verbatimFunction(size_t _arguments, size_t _returnVariables) const
{
	yulAssert(_arguments <= verbatimMaxInputSlots);
	yulAssert(_returnVariables <= verbatimMaxOutputSlots);

	auto const verbatimIndex = toContinuousVerbatimIndex(_arguments, _returnVariables);
	yulAssert(verbatimIndex < verbatimIDOffset);

	if (
		auto& verbatimFunctionPtr = m_verbatimFunctions[verbatimIndex];
		!verbatimFunctionPtr
	)
		verbatimFunctionPtr = std::make_unique<BuiltinFunctionForEVM>(createVerbatimFunction(_arguments, _returnVariables));

	return {verbatimIndex};
}
