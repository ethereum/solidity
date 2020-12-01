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

#include <boost/range/adaptor/reversed.hpp>


using namespace std;
using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::langutil;


void NoOutputAssembly::appendInstruction(evmasm::Instruction _instr)
{
	m_stackHeight += instructionInfo(_instr).ret - instructionInfo(_instr).args;
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
	yulAssert(!m_evm15, "Cannot use plain label references in EMV1.5 mode.");
	appendInstruction(evmasm::pushInstruction(1));
}

NoOutputAssembly::LabelID NoOutputAssembly::newLabelId()
{
	return 1;
}

AbstractAssembly::LabelID NoOutputAssembly::namedLabel(string const&)
{
	return 1;
}

void NoOutputAssembly::appendLinkerSymbol(string const&)
{
	yulAssert(false, "Linker symbols not yet implemented.");
}

void NoOutputAssembly::appendJump(int _stackDiffAfter, JumpType)
{
	yulAssert(!m_evm15, "Plain JUMP used for EVM 1.5");
	appendInstruction(evmasm::Instruction::JUMP);
	m_stackHeight += _stackDiffAfter;
}

void NoOutputAssembly::appendJumpTo(LabelID _labelId, int _stackDiffAfter, JumpType _jumpType)
{
	if (m_evm15)
		m_stackHeight += _stackDiffAfter;
	else
	{
		appendLabelReference(_labelId);
		appendJump(_stackDiffAfter, _jumpType);
	}
}

void NoOutputAssembly::appendJumpToIf(LabelID _labelId, JumpType)
{
	if (m_evm15)
		m_stackHeight--;
	else
	{
		appendLabelReference(_labelId);
		appendInstruction(evmasm::Instruction::JUMPI);
	}
}

void NoOutputAssembly::appendBeginsub(LabelID, int _arguments)
{
	yulAssert(m_evm15, "BEGINSUB used for EVM 1.0");
	yulAssert(_arguments >= 0, "");
	m_stackHeight += _arguments;
}

void NoOutputAssembly::appendJumpsub(LabelID, int _arguments, int _returns)
{
	yulAssert(m_evm15, "JUMPSUB used for EVM 1.0");
	yulAssert(_arguments >= 0 && _returns >= 0, "");
	m_stackHeight += _returns - _arguments;
}

void NoOutputAssembly::appendReturnsub(int _returns, int _stackDiffAfter)
{
	yulAssert(m_evm15, "RETURNSUB used for EVM 1.0");
	yulAssert(_returns >= 0, "");
	m_stackHeight += _stackDiffAfter - _returns;
}

void NoOutputAssembly::appendAssemblySize()
{
	appendInstruction(evmasm::Instruction::PUSH1);
}

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> NoOutputAssembly::createSubAssembly()
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
		fun.second.generateCode = [=](FunctionCall const& _call, AbstractAssembly& _assembly, BuiltinContext&, std::function<void(Expression const&)> _visitExpression)
		{
			size_t visited = 0;
			for (size_t j = 0; j < _call.arguments.size(); j++)
			{
				size_t const i = _call.arguments.size() - j - 1;
				if (!fun.second.literalArgument(i))
				{
					_visitExpression(_call.arguments[i]);
					visited++;
				}
			}

			for (size_t i = 0; i < visited; i++)
				_assembly.appendInstruction(evmasm::Instruction::POP);

			for (size_t i = 0; i < returns; i++)
				_assembly.appendConstant(u256(0));
		};
	}
}
