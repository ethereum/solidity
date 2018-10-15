/*(
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

#include <libyul/optimiser/Metrics.h>

#include <libsolidity/inlineasm/AsmData.h>

using namespace dev;
using namespace dev::julia;

size_t CodeSize::codeSize(Statement const& _statement)
{
	CodeSize cs;
	cs.visit(_statement);
	return cs.m_size;
}

size_t CodeSize::codeSize(Expression const& _expression)
{
	CodeSize cs;
	cs.visit(_expression);
	return cs.m_size;
}

void CodeSize::visit(Statement const& _statement)
{
	++m_size;
	ASTWalker::visit(_statement);
}

void CodeSize::visit(Expression const& _expression)
{
	++m_size;
	ASTWalker::visit(_expression);
}
