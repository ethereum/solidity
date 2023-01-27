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
 * Assembly interface that ignores everything. Can be used as a backend for a compilation dry-run.
 */

#include <libyul/backends/evm/NoOutputAssembly.h>

#include <libyul/AST.h>
#include <libyul/Exceptions.h>

#include <libevmasm/Instruction.h>

#include <range/v3/view/iota.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;


void NoOutputAssembly::appendInstruction(evmasm::Instruction _instr)
{
	m_stackHeight += instructionInfo(_instr, m_evmVersion).ret - instructionInfo(_instr, m_evmVersion).args;
}

void NoOutputAssembly::appendConstant(u256 const&)
{
	appendInstruction(evmasm::pushInstruction(1));
}

void NoOutputAssembly::appendLabel(LabelID)
{
	appendInstruction(evmasm::Instruction::JUMPDEST);
}

void NoOutputAssembly::appendLabelReference(LabelID)
{
	appendInstruction(evmasm::pushInstruction(1));
}

NoOutputAssembly::LabelID NoOutputAssembly::newLabelId()
{
	return 1;
}

AbstractAssembly::LabelID NoOutputAssembly::namedLabel(string const&, size_t, size_t, optional<size_t>)
{
	return 1;
}

void NoOutputAssembly::appendLinkerSymbol(string const&)
{
	yulAssert(false, "Linker symbols not yet implemented.");
}

void NoOutputAssembly::appendVerbatim(bytes, size_t _arguments, size_t _returnVariables)
{
	m_stackHeight += static_cast<int>(_returnVariables - _arguments);
}

void NoOutputAssembly::appendJump(int _stackDiffAfter, JumpType)
{
	appendInstruction(evmasm::Instruction::JUMP);
	m_stackHeight += _stackDiffAfter;
}

void NoOutputAssembly::appendJumpTo(LabelID _labelId, int _stackDiffAfter, JumpType _jumpType)
{
	appendLabelReference(_labelId);
	appendJump(_stackDiffAfter, _jumpType);
}

void NoOutputAssembly::appendJumpToIf(LabelID _labelId, JumpType)
{
	appendLabelReference(_labelId);
	appendInstruction(evmasm::Instruction::JUMPI);
}

void NoOutputAssembly::appendAssemblySize()
{
	appendInstruction(evmasm::Instruction::PUSH1);
}

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> NoOutputAssembly::createSubAssembly(bool, std::string)
{
	yulAssert(false, "Sub assemblies not implemented.");
	return {};
}

void NoOutputAssembly::appendDataOffset(std::vector<AbstractAssembly::SubID> const&)
{
	appendInstruction(evmasm::Instruction::PUSH1);
}

void NoOutputAssembly::appendDataSize(std::vector<AbstractAssembly::SubID> const&)
{
	appendInstruction(evmasm::Instruction::PUSH1);
}

AbstractAssembly::SubID NoOutputAssembly::appendData(bytes const&)
{
	return 1;
}


void NoOutputAssembly::appendImmutable(std::string const&)
{
	yulAssert(false, "loadimmutable not implemented.");
}

void NoOutputAssembly::appendImmutableAssignment(std::string const&)
{
	yulAssert(false, "setimmutable not implemented.");
}

NoOutputEVMDialect::NoOutputEVMDialect(EVMDialect const& _copyFrom):
	EVMDialect(_copyFrom.evmVersion(), _copyFrom.providesObjectAccess())
{
	for (auto& fun: m_functions)
	{
		size_t returns = fun.second.returns.size();
		fun.second.generateCode = [=](FunctionCall const& _call, AbstractAssembly& _assembly, BuiltinContext&)
		{
			for (size_t i: ranges::views::iota(0u, _call.arguments.size()))
				if (!fun.second.literalArgument(i))
					_assembly.appendInstruction(evmasm::Instruction::POP);

			for (size_t i = 0; i < returns; i++)
				_assembly.appendConstant(u256(0));
		};
	}
}
