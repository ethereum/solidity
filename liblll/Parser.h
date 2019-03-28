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
/** @file Parser.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <liblll/Exceptions.h>
#include <libdevcore/Common.h>
#include <string>
#include <vector>

namespace boost { namespace spirit { class utree; } }
namespace sp = boost::spirit;

namespace dev
{
namespace lll
{

void killBigints(sp::utree const& _this);
void parseTreeLLL(std::string const& _s, sp::utree& o_out);
void debugOutAST(std::ostream& _out, sp::utree const& _this);

}
}
