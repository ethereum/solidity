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

#include <libyul/AsmAnalysisInfo.h>

namespace solidity::yul
{

struct StackLayout
{
	struct BlockInfo
	{
		Stack entry;
	};
	std::map<Block const*, BlockInfo> blockInfos;
	std::map<Statement const*, Stack> statementInfos;
};

class DirectStackLayoutGenerator: public ASTWalker
{
public:
	struct FunctionInfo {
		std::shared_ptr<DebugData const> debugData;
		Scope::Function const& function;
		std::vector<VariableSlot> parameters;
		std::vector<VariableSlot> returnVariables;
	};
	struct Context {
		StackLayout layout;
		std::vector<Scope::Function const*> functionList;
		std::map<Scope::Function const*, FunctionInfo> functionInfo;
	};
	static Context run(AsmAnalysisInfo const& _analysisInfo, Dialect const& _dialect, Block const& _ast);

	void operator()(Literal const&) override;
	void operator()(Identifier const&) override;
	void operator()(FunctionCall const& _funCall) override;
	void operator()(ExpressionStatement const& _statement) override;
	void operator()(Assignment const& _assignment) override;
	void operator()(VariableDeclaration const& _varDecl) override;
	void operator()(If const& _if) override;
	void operator()(Switch const& _switch) override;
	void operator()(ForLoop const&) override;
	void operator()(FunctionDefinition const&) override;
	void operator()(Break const&) override;
	void operator()(Continue const&) override;
	void operator()(Leave const&) override;
	void operator()(Block const& _block) override;

	void visit(Statement const& _stmt) override;
	using ASTWalker::visit;


private:
	DirectStackLayoutGenerator(Context& _context, AsmAnalysisInfo const& _info, Dialect const& _dialect, Stack _prefix = {}):
	m_context(_context), m_info(_info), m_dialect(_dialect), m_prefix(std::move(_prefix)) {}

	size_t visitFunctionCall(FunctionCall const&);
	void visitAssignmentOrDeclaration(
		std::shared_ptr<DebugData const> _debugData,
		std::vector<VariableSlot> const& _variables,
		Expression const* _expression
	);
	Scope::Function const& lookupFunction(YulString _name) const;
	Scope::Variable const& lookupVariable(YulString _name) const;
	void registerFunction(FunctionDefinition const& _function);

	Context& m_context;
	AsmAnalysisInfo const& m_info;
	Dialect const& m_dialect;
	Scope* m_scope = nullptr;
	Stack m_prefix;
	Stack m_stack;
};

}
