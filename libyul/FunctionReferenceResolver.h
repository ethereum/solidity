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

#include <libyul/optimiser/ASTWalker.h>

namespace solidity::yul
{

/**
 * Resolves references to user-defined functions in function calls.
 * Assumes the code is correct, i.e. does not check for references to be valid or unique.
 *
 * Be careful not to iterate over the result - it is not deterministic.
 */
class FunctionReferenceResolver: private ASTWalker
{
public:
	explicit FunctionReferenceResolver(Block const& _ast);
	std::map<FunctionCall const*, FunctionDefinition const*> const& references() const { return m_functionReferences; }

private:
	using ASTWalker::operator();
	void operator()(FunctionCall const& _functionCall) override;
	void operator()(Block const& _block) override;

	std::map<FunctionCall const*, FunctionDefinition const*> m_functionReferences;
	std::vector<std::map<YulString, FunctionDefinition const*>> m_scopes;
};


}
