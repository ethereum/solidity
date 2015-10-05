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
#include <utility>
#include <libdevcore/Exceptions.h>
#include <libevmasm/SourceLocation.h>

namespace dev
{
namespace solidity
{

class Error: virtual public Exception
{
public:
	enum class Type
	{
		DeclarationError,
		DocstringParsingError,
		ParserError,
		TypeError,

		Warning
	};

	Error(Type _type) :	m_type(_type)
	{
		switch(m_type)
		{
			case Type::DeclarationError:
				m_typeName = "Declaration Error";
				break;
			case Type::DocstringParsingError:
				m_typeName = "Docstring Parsing Error";
				break;
			case Type::ParserError:
				m_typeName = "Parser Error";
				break;
			case Type::TypeError:
				m_typeName = "Type Error";
				break;
			case Type::Warning:
				m_typeName = "Warning";
				break;
			default:
				m_typeName = "Error";
				break;
		}
	}

	Type const type() { return m_type; } const
	std::string const& typeName() const { return m_typeName; }

private:
	Type m_type;
	std::string m_typeName;
};

struct CompilerError: virtual Exception {};
struct InternalCompilerError: virtual Exception {};
struct FatalError: virtual Exception {};

using errorSourceLocationInfo = std::pair<std::string, SourceLocation>;

class SecondarySourceLocation
{
public:
	SecondarySourceLocation& append(std::string const& _errMsg, SourceLocation const& _sourceLocation)
	{
		infos.push_back(std::make_pair(_errMsg, _sourceLocation));
		return *this;
	}
	std::vector<errorSourceLocationInfo> infos;
};


using ErrorList = std::vector<std::shared_ptr<Error const>>;
using errinfo_sourceLocation = boost::error_info<struct tag_sourceLocation, SourceLocation>;
using errinfo_secondarySourceLocation = boost::error_info<struct tag_secondarySourceLocation, SecondarySourceLocation>;

}
}
