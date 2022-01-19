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
 * Stack layout generator for Yul to EVM code generation.
 */

#pragma once

#include <libyul/backends/evm/ControlFlowGraph.h>

#include <libyul/optimiser/ASTWalker.h>

namespace solidity::yul
{

struct StackLayout
{
	struct StackInfo
	{
		Stack entry;
		Stack exit;
	};
	std::map<Block const*, StackInfo> blockInfos;
	std::map<Statement const*, StackInfo> statementInfos;
};

class DirectStackLayoutGenerator: public ASTWalker
{
public:
	static void run(Block const& _ast);

	virtual void operator()(Literal const&) {}
	virtual void operator()(Identifier const&) {}
	virtual void operator()(FunctionCall const& _funCall);
	virtual void operator()(ExpressionStatement const& _statement);
	virtual void operator()(Assignment const& _assignment);
	virtual void operator()(VariableDeclaration const& _varDecl);
	virtual void operator()(If const& _if);
	virtual void operator()(Switch const& _switch);
	virtual void operator()(ForLoop const&);
	virtual void operator()(FunctionDefinition const&);
	virtual void operator()(Break const&);
	virtual void operator()(Continue const&);
	virtual void operator()(Leave const&);
	virtual void operator()(Block const& _block);

private:
	DirectStackLayoutGenerator() {}
	Stack m_stack;
};

}
