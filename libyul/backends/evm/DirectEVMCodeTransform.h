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
 * Code generator for translating Yul / inline assembly to EVM.
 */
#pragma once

#include <libyul/backends/evm/AbstractAssembly.h>
#include <libyul/backends/evm/DirectStackLayoutGenerator.h>
#include <libyul/backends/evm/EVMDialect.h>

namespace solidity::yul
{

class DirectEVMCodeTransform: public ASTWalker
{
public:
	/// Use named labels for functions 1) Yes and check that the names are unique
	/// 2) For none of the functions 3) for the first function of each name.
	enum class UseNamedLabels { YesAndForceUnique, Never, ForFirstFunctionOfEachName };

	static void run(
		AbstractAssembly& _assembly,
		BuiltinContext& _builtinContext,
		UseNamedLabels _useNamedLabelsForFunctions,
		AsmAnalysisInfo const& _analysisInfo,
		Dialect const& _dialect,
		Block const& _ast
	);

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
	DirectEVMCodeTransform(
		AbstractAssembly& _assembly,
		BuiltinContext& _builtinContext,
		UseNamedLabels _useNamedLabelsForFunctions,
		DirectStackLayoutGenerator::Context const& _context,
		AsmAnalysisInfo const& _analysisInfo,
		Dialect const& _dialect
	):
	m_assembly(_assembly),
	m_builtinContext(_builtinContext),
	m_useNamedLabelsForFunctions(_useNamedLabelsForFunctions),
	m_context(_context),
	m_info(_analysisInfo),
	m_dialect(_dialect)
	{}

	/// Shuffles m_stack to the desired @a _targetStack while emitting the shuffling code to m_assembly.
	/// Sets the source locations to the one in @a _debugData.
	void createStackLayout(std::shared_ptr<DebugData const> _debugData, Stack _targetStack);

	AbstractAssembly& m_assembly;
	BuiltinContext& m_builtinContext;
	UseNamedLabels m_useNamedLabelsForFunctions;
	DirectStackLayoutGenerator::Context const& m_context;
	AsmAnalysisInfo const& m_info;
	Dialect const& m_dialect;
	Stack m_stack;
	CFG::FunctionInfo const* m_currentFunctionInfo = nullptr;
	std::map<yul::FunctionCall const*, AbstractAssembly::LabelID> m_returnLabels;
	std::map<CFG::BasicBlock const*, AbstractAssembly::LabelID> m_blockLabels;
	std::map<CFG::FunctionInfo const*, AbstractAssembly::LabelID> const m_functionLabels;
	std::vector<StackTooDeepError> m_stackErrors;
};

}