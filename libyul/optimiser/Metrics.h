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
 * Module providing metrics for the optimizer.
 */

#pragma once

#include <libyul/optimiser/ASTWalker.h>

namespace dev
{
namespace yul
{

class CodeSize: public ASTWalker
{
public:
	/// Returns a metric for the code size of an AST element.
	/// More specifically, it returns the number of AST nodes.
	static size_t codeSize(Statement const& _statement);
	/// Returns a metric for the code size of an AST element.
	/// More specifically, it returns the number of AST nodes.
	static size_t codeSize(Expression const& _expression);

private:
	virtual void visit(Statement const& _statement) override;
	virtual void visit(Expression const& _expression) override;

private:
	size_t m_size = 0;
};

}
}
