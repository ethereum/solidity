// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libsolutil/Common.h>

namespace solidity::yul
{

/**
 * Structural simplifier. Performs the following simplification steps:
 * - replace if with true condition with its body
 * - remove if with false condition
 * - replace switch with const expr with matching case body
 * - replace for with false condition by its initialization part
 *
 * The LiteralRematerialiser should be run before this.
 *
 * Prerequisite: Disambiguator.
 *
 * Important: Can only be used on EVM code.
 */
class StructuralSimplifier: public ASTModifier
{
public:
	static constexpr char const* name{"StructuralSimplifier"};
	static void run(OptimiserStepContext&, Block& _ast);

	using ASTModifier::operator();
	void operator()(Block& _block) override;
private:
	StructuralSimplifier() = default;

	void simplify(std::vector<Statement>& _statements);
	bool expressionAlwaysTrue(Expression const& _expression);
	bool expressionAlwaysFalse(Expression const& _expression);
	std::optional<u256> hasLiteralValue(Expression const& _expression) const;
};

}
