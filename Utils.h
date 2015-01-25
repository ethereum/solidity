/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Solidity Utilities.
 */

#pragma once

#include <string>
#include <libsolidity/Exceptions.h>

namespace dev
{
namespace solidity
{

/// Assertion that throws an InternalCompilerError containing the given description if it is not met.
#define solAssert(CONDITION, DESCRIPTION) \
	::dev::solidity::solAssertAux(CONDITION, DESCRIPTION, __LINE__, __FILE__, ETH_FUNC)

inline void solAssertAux(bool _condition, std::string const& _errorDescription, unsigned _line,
						 char const* _file, char const* _function)
{
	if (!_condition)
		::boost::throw_exception( InternalCompilerError()
				<< errinfo_comment(_errorDescription)
				<< ::boost::throw_function(_function)
				<< ::boost::throw_file(_file)
				<< ::boost::throw_line(_line));
}

inline void solAssertAux(void const* _pointer, std::string const& _errorDescription, unsigned _line,
						 char const* _file, char const* _function)
{
	solAssertAux(_pointer != nullptr, _errorDescription, _line, _file, _function);
}

}
}
