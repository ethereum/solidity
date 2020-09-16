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
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/Dialect.h>

// because of instruction
#include <libyul/backends/evm/EVMDialect.h>

#include <map>

namespace solidity::smtutil
{
class SolverInterface;
class Expression;
struct Sort;
}

namespace solidity::yul
{

/**
 * Reasoning-based simplifier.
 * This optimizer uses SMT solvers to check whether `if` conditions are constant.
 * - If `constraints AND condition` is UNSAT, the condition is never true and the whole body can be removed.
 * - If `constraints AND NOT condition` is UNSAT, the condition is always true and can be replaced by `1`.
 * The simplifications above can only be applied if the condition is movable.
 *
 * It is only effective on the EVM dialect, but safe to use on other dialects.
 *
 * Prerequisite: Disambiguator, SSATransform.
 */
class ReasoningBasedSimplifier: public ASTModifier
{
public:
	static constexpr char const* name{"ReasoningBasedSimplifier"};
	static void run(OptimiserStepContext& _context, Block& _ast);
	static std::optional<std::string> invalidInCurrentEnvironment();

	using ASTModifier::operator();
	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(If& _if) override;

private:
	explicit ReasoningBasedSimplifier(
		Dialect const& _dialect,
		std::set<YulString> const& _ssaVariables
	);

	smtutil::Expression encodeExpression(
		Expression const& _expression
	);

	virtual smtutil::Expression encodeEVMBuiltin(
		evmasm::Instruction _instruction,
		std::vector<Expression> const& _arguments
	);

	smtutil::Expression int2bv(smtutil::Expression _arg) const;
	smtutil::Expression bv2int(smtutil::Expression _arg) const;

	smtutil::Expression newVariable();
	virtual smtutil::Expression newRestrictedVariable();
	std::string uniqueName();

	virtual std::shared_ptr<smtutil::Sort> defaultSort() const;
	virtual smtutil::Expression booleanValue(smtutil::Expression _value) const;
	virtual smtutil::Expression constantValue(size_t _value) const;
	virtual smtutil::Expression literalValue(Literal const& _literal) const;
	virtual smtutil::Expression unsignedToSigned(smtutil::Expression _value);
	virtual smtutil::Expression signedToUnsigned(smtutil::Expression _value);
	virtual smtutil::Expression wrap(smtutil::Expression _value);

	Dialect const& m_dialect;
	std::set<YulString> const& m_ssaVariables;
	std::unique_ptr<smtutil::SolverInterface> m_solver;
	std::map<YulString, smtutil::Expression> m_variables;

	size_t m_varCounter = 0;
};

}
