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

#pragma once

#include <string>
#include <libdevcore/Exceptions.h>
#include <libsolidity/BaseTypes.h>

namespace dev
{
namespace solidity
{

class ParserError: public virtual Exception
{
public:
	ParserError(int _position, std::string const& _description):
		m_position(_position), m_description(_description) {}
	virtual const char* what() const noexcept;
	int getPosition() const { return m_position; }

private:
	int m_position;
	std::string m_description;
};

class TypeError: public virtual Exception
{
public:
	TypeError(Location const& _location, std::string const& _description):
		m_location(_location), m_description(_description) {}
	virtual const char* what() const noexcept;
	Location const& getLocation() const { return m_location; }

private:
	Location m_location;
	std::string m_description;
};

class DeclarationError: public virtual Exception
{
public:
	DeclarationError(Location const& _location, std::string const& _description):
		m_location(_location), m_description(_description) {}
	virtual const char* what() const noexcept;
	Location const& getLocation() const { return m_location; }

private:
	Location m_location;
	std::string m_description;
};

}
}
