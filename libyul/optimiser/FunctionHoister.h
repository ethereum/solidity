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
 * Optimiser component that changes the code so that all function definitions are at the top
 * level block.
 */

#pragma once

#include <libyul/ASTDataForward.h>
#include <libyul/optimiser/ASTWalker.h>

namespace dev
{
namespace yul
{

/**
 * Moves all functions to the top-level scope.
 * Applying this transformation to source code that has ambiguous identifiers might
 * lead to invalid code.
 *
 * Prerequisites: Disambiguator
 */
class FunctionHoister: public ASTModifier
{
public:
	using ASTModifier::operator();
	virtual void operator()(Block& _block);

private:
	bool m_isTopLevel = true;
	std::vector<Statement> m_functions;
};

}
}
