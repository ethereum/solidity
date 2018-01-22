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
 * Utility module that provides the list of builtin functions.
 */

#pragma once

#include <set>
#include <string>

namespace dev
{
namespace julia
{

class BuiltinFunctions
{
public:
	explicit BuiltinFunctions(bool _iulia = false);

	static std::set<std::string> const& names(bool _iulia = false);

private:
	static BuiltinFunctions const& instance(bool _iulia = false);

	std::set<std::string> m_builtinFunctions;
};

}
}
