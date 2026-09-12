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

#include <libevmasm/AssemblyItem.h>
#include <libevmasm/Instruction.h>
#include <libevmasm/SemanticInformation.h>

#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/Object.h>
#include <libyul/Utilities.h>

#include <libsolutil/StringUtils.h>

#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/to_container.hpp>

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

bool isLowLevelStackManipulationInstruction(evmasm::Instruction const& _instruction)
{
	return
		evmasm::SemanticInformation::isSwapInstruction(_instruction) ||
		evmasm::SemanticInformation::isDupInstruction(_instruction) ||
		isPushInstruction(_instruction);
}

bool isLowLevelControlFlowInstruction(evmasm::Instruction const& _instruction)
{
	switch (_instruction)
	{
	case evmasm::Instruction::JUMP:
	case evmasm::Instruction::JUMPI:
	case evmasm::Instruction::JUMPDEST:
	case evmasm::Instruction::CALLSUB:
	case evmasm::Instruction::CALLDEST:
	case evmasm::Instruction::RETURNSUB:
		return true;
	default:
		return false;
	}
}

std::set<std::string, std::less<>> createReservedIdentifiers(langutil::EVMVersion _evmVersion)
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
	// TODO remove this in 0.9.0. We allow creating functions or identifiers in Yul with the name
	// clz for VMs before osaka.
	auto clzException = [&](evmasm::Instruction _instr) -> bool
	{
		return _instr == evmasm::Instruction::CLZ && !_evmVersion.hasCLZ();
	};

	// TODO remove this in 0.9.0. We allow creating functions or identifiers in Yul with the name
	// slotnum for VMs before amsterdam.
	auto slotNumException = [&](evmasm::Instruction _instr) -> bool
	{
		return _instr == evmasm::Instruction::SLOTNUM && !_evmVersion.hasSlotNum();
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
			!transientStorageException(instr.second) &&
			!clzException(instr.second) &&
			!slotNumException(instr.second)
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

	return reserved;
}

std::vector<BuiltinFunctionForEVM const*> createDialectBuiltins(
	std::vector<std::tuple<EVMBuiltins::Scopes, BuiltinFunctionForEVM>> const& _allBuiltins,
	langutil::EVMVersion const _evmVersion,
	bool const _objectAccess
)
{
	std::vector<BuiltinFunctionForEVM const*> builtins;
	builtins.reserve(_allBuiltins.size());

	for (auto const& [scopes, builtin]: _allBuiltins)
	{
		bool builtinShouldBeAdded = true;
		if (scopes.instruction())
		{
			if (scopes.replaced())
				builtinShouldBeAdded = false;
			else
			{
				// Exclude prevrandao as builtin for VMs before paris and difficulty for VMs after paris.
				auto prevRandaoException = [&](std::string_view const _instrName) -> bool
				{
					return (_instrName == "prevrandao" && _evmVersion < langutil::EVMVersion::paris()) || (_instrName == "difficulty" && _evmVersion >= langutil::EVMVersion::paris());
				};

				yulAssert(builtin.instruction);
				auto const& _opcode = *builtin.instruction;
				builtinShouldBeAdded =
					!isLowLevelControlFlowInstruction(_opcode) &&
					!isLowLevelStackManipulationInstruction(_opcode) &&
					_evmVersion.hasOpcode(_opcode) &&
					!prevRandaoException(builtin.name);
			}
		}

		builtinShouldBeAdded &= !scopes.requiresObjectAccess() || _objectAccess;

		if (builtinShouldBeAdded)
			builtins.emplace_back(&builtin);
		else
			builtins.emplace_back(nullptr);
	}

	return builtins;
}

std::regex const& verbatimPattern()
{
	std::regex static const pattern{"([1-9]?[0-9])i_([1-9]?[0-9])o"};
	return pattern;
}

}

EVMDialect::EVMDialect(langutil::EVMVersion _evmVersion, bool _objectAccess):
	m_objectAccess(_objectAccess),
	m_evmVersion(_evmVersion),
	m_functions(createDialectBuiltins(allBuiltins().functions(), _evmVersion, _objectAccess)),
	m_reserved(createReservedIdentifiers(_evmVersion))
{
	for (auto const& [index, maybeBuiltin]: m_functions | ranges::views::enumerate)
		if (maybeBuiltin)
			// ids are offset by the maximum number of verbatim functions
			m_builtinFunctionsByName[maybeBuiltin->name] = BuiltinHandle{index + verbatimIDOffset};

	m_discardFunction = EVMDialect::findBuiltin("pop");
	m_equalityFunction = EVMDialect::findBuiltin("eq");
	m_booleanNegationFunction = EVMDialect::findBuiltin("iszero");
	m_memoryStoreFunction = EVMDialect::findBuiltin("mstore");
	m_memoryLoadFunction = EVMDialect::findBuiltin("mload");
	m_storageStoreFunction = EVMDialect::findBuiltin("sstore");
	m_storageLoadFunction = EVMDialect::findBuiltin("sload");
	m_hashFunction = EVMDialect::findBuiltin("keccak256");

	m_auxiliaryBuiltinHandles.add = EVMDialect::findBuiltin("add");
	m_auxiliaryBuiltinHandles.exp = EVMDialect::findBuiltin("exp");
	m_auxiliaryBuiltinHandles.mul = EVMDialect::findBuiltin("mul");
	m_auxiliaryBuiltinHandles.not_ = EVMDialect::findBuiltin("not");
	m_auxiliaryBuiltinHandles.shl = EVMDialect::findBuiltin("shl");
	m_auxiliaryBuiltinHandles.sub = EVMDialect::findBuiltin("sub");
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
	auto const* maybeBuiltin = m_functions[_handle.id - verbatimIDOffset];
	yulAssert(maybeBuiltin);
	return *maybeBuiltin;
}

bool EVMDialect::reservedIdentifier(std::string_view _name) const
{
	if (m_objectAccess)
		if (_name.substr(0, "verbatim"s.size()) == "verbatim")
			return true;
	return m_reserved.contains(_name);
}

EVMDialect const& EVMDialect::strictAssemblyForEVM(langutil::EVMVersion _evmVersion)
{
	static std::map<langutil::EVMVersion, std::unique_ptr<EVMDialect const>> dialects;
	static YulStringRepository::ResetCallback callback{[&] { dialects.clear(); }};
	if (!dialects[_evmVersion])
		dialects[_evmVersion] = std::make_unique<EVMDialect>(_evmVersion, false);
	return *dialects[_evmVersion];
}

EVMDialect const& EVMDialect::strictAssemblyForEVMObjects(langutil::EVMVersion _evmVersion)
{
	static std::map<langutil::EVMVersion, std::unique_ptr<EVMDialect const>> dialects;
	static YulStringRepository::ResetCallback callback{[&] { dialects.clear(); }};
	if (!dialects[_evmVersion])
		dialects[_evmVersion] = std::make_unique<EVMDialect>(_evmVersion, true);
	return *dialects[_evmVersion];
}

std::set<std::string_view> EVMDialect::builtinFunctionNames() const
{
	return ranges::views::keys(m_builtinFunctionsByName) | ranges::to<std::set>;
}

BuiltinFunctionForEVM EVMDialect::createVerbatimFunctionFromHandle(BuiltinHandle const& _handle)
{
	return std::apply(EVMBuiltins::createVerbatimFunction, verbatimIndexToArgsAndRets(_handle.id));
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
		verbatimFunctionPtr = std::make_unique<BuiltinFunctionForEVM>(EVMBuiltins::createVerbatimFunction(_arguments, _returnVariables));

	return {verbatimIndex};
}

EVMBuiltins const& EVMDialect::allBuiltins()
{
	static EVMBuiltins const builtins;
	return builtins;
}
