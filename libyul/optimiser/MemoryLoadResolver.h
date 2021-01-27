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


#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/Scoper.h>
#include <libyul/optimiser/ASTWalker.h>

#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/SideEffects.h>
#include <libyul/AST.h>
#include <libyul/Dialect.h>
#include <libyul/YulString.h>

#include <libsmtutil/SMTPortfolio.h>
#include <libsmtutil/SolverInterface.h>

#include <set>

namespace solidity::yul
{

class MemoryLoadResolver: public Scoper
{

public:
	static constexpr char const* name{"MemoryLoadResolver"};
	static void run(OptimiserStepContext& _context, Block& _ast);

	using ASTModifier::operator();
	void operator()(VariableDeclaration& _variableDeclaration) override;
	void operator()(FunctionCall& _functionCall) override;
	void operator()(If& _if) override;
	void operator()(Switch& _switch) override;
	void operator()(ForLoop& _forLoop) override;
	void operator()(FunctionDefinition& _functionDefinition) override;

	using ASTModifier::visit;
	void visit(Expression& _e) override;

private:
	MemoryLoadResolver(
		Dialect const& _dialect,
		std::map<YulString, SideEffects> _functionSideEffects,
		std::set<YulString> _ssaVariables,
		bool _containsMsize
	) :
		m_functionSideEffects(std::move(_functionSideEffects)),
		m_ssaVariables(std::move(_ssaVariables)),
		m_containsMsize(_containsMsize),
		m_dialect(_dialect),
		m_solver(std::make_unique<smtutil::SMTPortfolio>())
	{
		for (auto const& name: m_ssaVariables)
			std::cout << name.str() << std::endl;
	}

	/// The mapping for memory
	std::map<YulString, YulString> m_memory;

	/// Side-effects of user-defined functions. Worst-case side-effects are assumed
	/// if this is not provided or the function is not found.
	std::map<YulString, SideEffects> const m_functionSideEffects;

	/// A set of all SSA variables in the AST
	std::set<YulString> const m_ssaVariables;

	// TODO is this needed? loadresolver seems to use it
	bool m_containsMsize = true;

	Dialect const& m_dialect;

	/// Joins the `_older` and current memory during control flow joins
	void joinMemory(std::map<YulString, YulString> const& _older);

	void clearMemoryIfInvalidated(Block const& _block);

	void clearMemoryIfInvalidated(Expression const& _expr);

	/// Checks if a function call is identical to a mstore(x, y), where x and y are SSA variables.
	std::optional<std::pair<YulString, YulString>> isSimpleMStore(
		FunctionCall const& _functionCall
	) const;

	smtutil::Expression encodeExpression(
		Expression const& _expression
	);

	virtual smtutil::Expression encodeEVMBuiltin(
		evmasm::Instruction _instruction,
		std::vector<Expression> const& _arguments
	);

	smtutil::Expression newVariable();
	virtual smtutil::Expression newRestrictedVariable(bigint _maxValue = (bigint(1) << 256));
	std::string uniqueName();

	virtual std::shared_ptr<smtutil::Sort> defaultSort() const;
	virtual smtutil::Expression wrap(smtutil::Expression _value);

	virtual smtutil::Expression constantValue(size_t _value) const;
	virtual smtutil::Expression literalValue(Literal const& _literal) const;

	bool invalidatesMemoryLocation(YulString const& _name, Expression const& _expression);

	std::unique_ptr<smtutil::SolverInterface> m_solver;
	std::map<YulString, smtutil::Expression> m_variables;

	size_t m_varCounter = 0;

	// TODO have a variable for function side effects
};

}
