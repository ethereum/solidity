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
/**
 * Assembly interface that ignores everything. Can be used as a backend for a compilation dry-run.
 */

#include <libyul/backends/evm/NoOutputAssembly.h>

#include <libevmasm/Instruction.h>

#include <liblangutil/Exceptions.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;


void NoOutputAssembly::appendInstruction(solidity::Instruction _instr)
{
	m_stackHeight += solidity::instructionInfo(_instr).ret - solidity::instructionInfo(_instr).args;
}

void NoOutputAssembly::appendConstant(u256 const&)
{
	appendInstruction(solidity::pushInstruction(1));
}

void NoOutputAssembly::appendLabel(LabelID)
{
	appendInstruction(solidity::Instruction::JUMPDEST);
}

void NoOutputAssembly::appendLabelReference(LabelID)
{
	solAssert(!m_evm15, "Cannot use plain label references in EMV1.5 mode.");
	appendInstruction(solidity::pushInstruction(1));
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
	solAssert(false, "Linker symbols not yet implemented.");
}

void NoOutputAssembly::appendJump(int _stackDiffAfter)
{
	solAssert(!m_evm15, "Plain JUMP used for EVM 1.5");
	appendInstruction(solidity::Instruction::JUMP);
	m_stackHeight += _stackDiffAfter;
}

void NoOutputAssembly::appendJumpTo(LabelID _labelId, int _stackDiffAfter)
{
	if (m_evm15)
		m_stackHeight += _stackDiffAfter;
	else
	{
		appendLabelReference(_labelId);
		appendJump(_stackDiffAfter);
	}
}

void NoOutputAssembly::appendJumpToIf(LabelID _labelId)
{
	if (m_evm15)
		m_stackHeight--;
	else
	{
		appendLabelReference(_labelId);
		appendInstruction(solidity::Instruction::JUMPI);
	}
}

void NoOutputAssembly::appendBeginsub(LabelID, int _arguments)
{
	solAssert(m_evm15, "BEGINSUB used for EVM 1.0");
	solAssert(_arguments >= 0, "");
	m_stackHeight += _arguments;
}

void NoOutputAssembly::appendJumpsub(LabelID, int _arguments, int _returns)
{
	solAssert(m_evm15, "JUMPSUB used for EVM 1.0");
	solAssert(_arguments >= 0 && _returns >= 0, "");
	m_stackHeight += _returns - _arguments;
}

void NoOutputAssembly::appendReturnsub(int _returns, int _stackDiffAfter)
{
	solAssert(m_evm15, "RETURNSUB used for EVM 1.0");
	solAssert(_returns >= 0, "");
	m_stackHeight += _stackDiffAfter - _returns;
}

void NoOutputAssembly::appendAssemblySize()
{
	appendInstruction(solidity::Instruction::PUSH1);
}

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> NoOutputAssembly::createSubAssembly()
{
	solAssert(false, "Sub assemblies not implemented.");
	return {};
}

void NoOutputAssembly::appendDataOffset(AbstractAssembly::SubID)
{
	appendInstruction(solidity::Instruction::PUSH1);
}

void NoOutputAssembly::appendDataSize(AbstractAssembly::SubID)
{
	appendInstruction(solidity::Instruction::PUSH1);
}

AbstractAssembly::SubID NoOutputAssembly::appendData(bytes const&)
{
	return 1;
}

NoOutputEVMDialect::NoOutputEVMDialect(shared_ptr<EVMDialect> const& _copyFrom):
	EVMDialect(_copyFrom->flavour, _copyFrom->providesObjectAccess(), _copyFrom->evmVersion())
{
	for (auto& fun: m_functions)
	{
		size_t parameters = fun.second.parameters.size();
		size_t returns = fun.second.returns.size();
		fun.second.generateCode = [=](FunctionCall const&, AbstractAssembly& _asm, std::function<void()> _visitArguments)
		{
			_visitArguments();
			for (size_t i = 0; i < parameters; i++)
				_asm.appendInstruction(dev::solidity::Instruction::POP);

			for (size_t i = 0; i < returns; i++)
				_asm.appendConstant(u256(0));
		};
	}
}
