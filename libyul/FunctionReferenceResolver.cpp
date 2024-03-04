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

#include <libyul/FunctionReferenceResolver.h>

#include <libyul/AST.h>
#include <libsolutil/CommonData.h>

#include <range/v3/view/reverse.hpp>

using namespace solidity::yul;
using namespace solidity::util;

FunctionReferenceResolver::FunctionReferenceResolver(Block const& _ast)
{
	(*this)(_ast);
	yulAssert(m_scopes.empty());
}

void FunctionReferenceResolver::operator()(FunctionCall const& _functionCall)
{
	for (auto&& scope: m_scopes | ranges::views::reverse)
		if (FunctionDefinition const** function = util::valueOrNullptr(scope, _functionCall.functionName.name))
		{
			m_functionReferences[&_functionCall] = *function;
			break;
		}

	// If we did not find anything, it was a builtin call.

	ASTWalker::operator()(_functionCall);
}

void FunctionReferenceResolver::operator()(Block const& _block)
{
	m_scopes.emplace_back();
	for (auto const& statement: _block.statements)
		if (auto const* function = std::get_if<FunctionDefinition>(&statement))
			m_scopes.back()[function->name] = function;

	ASTWalker::operator()(_block);

	m_scopes.pop_back();
}
