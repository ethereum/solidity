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

namespace yul
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
