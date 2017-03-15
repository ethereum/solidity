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
/** @file CompilerState.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <boost/spirit/include/support_utree.hpp>
#include "CodeFragment.h"

namespace dev
{
namespace eth
{

struct Macro
{
	std::vector<std::string> args;
	boost::spirit::utree code;
	std::map<std::string, CodeFragment> env;
};

struct CompilerState
{
	CompilerState();

	CodeFragment const& getDef(std::string const& _s);
	void populateStandard();

	unsigned stackSize = 128;
	std::map<std::string, std::pair<unsigned, unsigned>> vars;       ///< maps name to stack offset & size.
	std::map<std::string, CodeFragment> defs;
	std::map<std::string, CodeFragment> args;
	std::map<std::string, CodeFragment> outers;
	std::map<std::pair<std::string, unsigned>, Macro> macros;
	std::vector<boost::spirit::utree> treesToKill;
	bool usedAlloc = false;
};

}
}
