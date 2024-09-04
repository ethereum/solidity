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

#include <libyul/backends/evm/ControlFlow.h>
#include <libyul/backends/evm/SSAControlFlowGraph.h>

namespace solidity::yul
{
class SSACFGValidator
{
public:
	static void validate(ControlFlow const& _controlFlow, Block const& _ast, AsmAnalysisInfo const& _analysisInfo, Dialect const& _dialect);
private:
    struct Context {
    	AsmAnalysisInfo const& analysisInfo;
	    Dialect const& dialect;
    	ControlFlow const& controlFlow;
    	SSACFG const& cfg;
    };
	SSACFGValidator(Context const& _context): m_context(_context)
	{}
	std::optional<std::vector<SSACFG::ValueId>> consumeExpression(Expression const& _expression);
	std::optional<SSACFG::ValueId> consumeUnaryExpression(Expression const& _expression)
	{
		if (auto result = consumeExpression(_expression))
		{
			yulAssert(result->size() == 1);
			return result->front();
		}
		else
			return std::nullopt;
	}
	bool consumeBlock(Block const& _block);
	bool consumeStatement(Statement const& _statement);
	SSACFG::BasicBlock const& currentBlock() const
	{
		return m_context.cfg.block(m_currentBlock);
	}
	Scope::Variable const* resolveVariable(YulName _name) const;
	Scope::Function const* resolveFunction(YulName _name) const;
	SSACFG::ValueId lookupIdentifier(Identifier const& _identifier) const;
	SSACFG::ValueId lookupLiteral(Literal const& _literal) const;
	/// @returns true if the call can continue, false otherwise
	bool validateCall(std::variant<SSACFG::BuiltinCall, SSACFG::Call> const& _kind, Identifier const& _functionName, size_t _numOutputs) const;

	SSACFG::BasicBlock::ConditionalJump const& expectConditionalJump() const;
	SSACFG::BasicBlock::Jump const& expectUnconditionalJump() const;
	/// Applys the phi functions of @a _target assuming an entry from @a _source.
	std::map<Scope::Variable const*, SSACFG::ValueId> applyPhis(SSACFG::BlockId _source, SSACFG::BlockId _target);

	Context const& m_context;
	Scope* m_scope = nullptr;
    SSACFG::BlockId m_currentBlock;
	size_t m_currentOperation = std::numeric_limits<size_t>::max();
	std::map<Scope::Variable const*, SSACFG::ValueId> m_currentVariableValues;
	struct LoopInfo
	{
		std::set<Scope::Variable const*> loopVariables;
		std::map<Scope::Variable const*, SSACFG::ValueId> loopExitVariableValues;
		SSACFG::BlockId exitBlock;
		std::optional<std::map<Scope::Variable const*, SSACFG::ValueId>> loopPostVariableValues;
		std::optional<SSACFG::BlockId> postBlock;
	};
	std::unique_ptr<LoopInfo> m_currentLoopInfo;
};
}