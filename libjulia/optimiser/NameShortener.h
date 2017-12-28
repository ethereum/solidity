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
 * Returns a copy of an AST with all names replaced by shortened versions.
 */

#pragma once

#include <libjulia/ASTDataForward.h>

#include <libjulia/optimiser/ASTCopier.h>

#include <libsolidity/inlineasm/AsmAnalysisInfo.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <set>

namespace dev
{
namespace julia
{

/**
 * Returns a copy of an AST with all names replaced by shortened versions.
 */
class NameShortener: public ASTCopier
{
public:
	explicit NameShortener(Block const& _ast, size_t _maxSize = 0);

protected:
	virtual std::string translateIdentifier(std::string const& _name) override;

private:
	std::map<std::string, std::string> m_translations;
};

}
}
