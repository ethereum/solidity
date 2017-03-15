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
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;

struct CompilerError: virtual Exception {};
struct InternalCompilerError: virtual Exception {};
struct FatalError: virtual Exception {};
struct UnimplementedFeatureError: virtual Exception{};

class Error: virtual public Exception
{
public:
	enum class Type
	{
		DeclarationError,
		DocstringParsingError,
		ParserError,
		TypeError,
		SyntaxError,
		Why3TranslatorError,
		Warning
	};

	explicit Error(
		Type _type,
		SourceLocation const& _location = SourceLocation(),
		std::string const& _description = std::string()
	);

	Error(Type _type, std::string const& _description, SourceLocation const& _location = SourceLocation());

	Type type() const { return m_type; }
	std::string const& typeName() const { return m_typeName; }

	/// helper functions
	static Error const* containsErrorOfType(ErrorList const& _list, Error::Type _type)
	{
		for (auto e: _list)
		{
			if (e->type() == _type)
				return e.get();
		}
		return nullptr;
	}
	static bool containsOnlyWarnings(ErrorList const& _list)
	{
		for (auto e: _list)
		{
			if (e->type() != Type::Warning)
				return false;
		}
		return true;
	}
private:
	Type m_type;
	std::string m_typeName;
};


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


using errinfo_sourceLocation = boost::error_info<struct tag_sourceLocation, SourceLocation>;
using errinfo_secondarySourceLocation = boost::error_info<struct tag_secondarySourceLocation, SecondarySourceLocation>;

}
}
