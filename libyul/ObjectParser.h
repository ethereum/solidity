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
// SPDX-License-Identifier: GPL-3.0
/**
 * Parser for Yul code and data object container.
 */

#pragma once

#include <libyul/Dialect.h>
#include <libyul/AsmParser.h>
#include <libyul/Object.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/ParserBase.h>

#include <libsolutil/Common.h>

#include <memory>

namespace solidity::langutil
{
class Scanner;
}

namespace solidity::yul
{

/**
 * Yul object parser. Invokes the inline assembly parser.
 */
class ObjectParser: public langutil::ParserBase
{
public:
	explicit ObjectParser(langutil::ErrorReporter& _errorReporter, Dialect const& _dialect, Parser::DebugAttributeCache::Ptr _cache = {}):
		ParserBase(_errorReporter), m_dialect(_dialect), m_cache(std::move(_cache)) {}

	/// Parses a Yul object.
	/// Falls back to code-only parsing if the source starts with `{`.
	/// @param _reuseScanner if true, do check for end of input after the last `}`.
	/// @returns an empty shared pointer on error.
	std::shared_ptr<Object> parse(std::shared_ptr<langutil::Scanner> const& _scanner, bool _reuseScanner);

private:
	std::optional<SourceNameMap> tryParseSourceNameMapping() const;
	std::shared_ptr<Object> parseObject(Object* _containingObject = nullptr);
	std::shared_ptr<AST> parseCode(std::optional<SourceNameMap> _sourceNames);
	std::shared_ptr<AST> parseBlock(std::optional<SourceNameMap> _sourceNames);
	void parseData(Object& _containingObject);

	/// Tries to parse a name that is non-empty and unique inside the containing object.
	std::string parseUniqueName(Object const* _containingObject);
	void addNamedSubObject(Object& _container, std::string_view _name, std::shared_ptr<ObjectNode> _subObject);

	Dialect const& m_dialect;
	Parser::DebugAttributeCache::Ptr m_cache;
};

}
