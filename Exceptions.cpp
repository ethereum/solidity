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
 * Solidity exception hierarchy.
 */

#include <libsolidity/Exceptions.h>


namespace dev
{
namespace solidity
{

const char* ParserError::what() const noexcept
{
	ETH_RETURN_STRING("Parser error: " + m_description);
}

const char* DeclarationError::what() const noexcept
{
	ETH_RETURN_STRING("Declaration error: " + m_description);
}

const char* TypeError::what() const noexcept
{
	ETH_RETURN_STRING("Type error: " + m_description);
}

}
}
