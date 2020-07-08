// SPDX-License-Identifier: GPL-3.0
#pragma once

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/Dialect.h>

namespace solidity::yul
{

/**
 * Reverses the transformation of ForLoopConditionIntoBody.
 *
 * For any movable ``c``, it turns
 *
 * for { ... } 1 { ... } {
 *   if iszero(c) { break }
 *   ...
 * }
 *
 * into
 *
 * for { ... } c { ... } {
 *   ...
 * }
 *
 * and it turns
 *
 * for { ... } 1 { ... } {
 *   if c { break }
 *   ...
 * }
 *
 * into
 *
 * for { ... } iszero(c) { ... } {
 *   ...
 * }
 *
 * The LiteralRematerialiser should be run before this step.
 */
class ForLoopConditionOutOfBody: public ASTModifier
{
public:
	static constexpr char const* name{"ForLoopConditionOutOfBody"};
	static void run(OptimiserStepContext&, Block& _ast);

	using ASTModifier::operator();
	void operator()(ForLoop& _forLoop) override;

private:
	ForLoopConditionOutOfBody(Dialect const& _dialect):
		m_dialect(_dialect)
	{}

	Dialect const& m_dialect;
};

}
