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
 * Parser for Yul code and data object container.
 */

#pragma once

#include <libyul/YulString.h>
#include <libyul/Object.h>
#include <libyul/Dialect.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/ParserBase.h>

#include <libdevcore/Common.h>

#include <memory>

namespace langutil
{
class Scanner;
}

namespace yul
{

/**
 * Yul object parser. Invokes the inline assembly parser.
 */
class ObjectParser: public langutil::ParserBase
{
public:
	explicit ObjectParser(
		langutil::ErrorReporter& _errorReporter,
		std::shared_ptr<Dialect> _dialect
	):
		ParserBase(_errorReporter), m_dialect(std::move(_dialect)) {}

	/// Parses a Yul object.
	/// Falls back to code-only parsing if the source starts with `{`.
	/// @param _reuseScanner if true, do check for end of input after the last `}`.
	/// @returns an empty shared pointer on error.
	std::shared_ptr<Object> parse(std::shared_ptr<langutil::Scanner> const& _scanner, bool _reuseScanner);

private:
	std::shared_ptr<Object> parseObject(Object* _containingObject = nullptr);
	std::shared_ptr<Block> parseCode();
	std::shared_ptr<Block> parseBlock();
	void parseData(Object& _containingObject);

	/// Tries to parse a name that is non-empty and unique inside the containing object.
	YulString parseUniqueName(Object const* _containingObject);
	void addNamedSubObject(Object& _container, YulString _name, std::shared_ptr<ObjectNode> _subObject);

	std::shared_ptr<Dialect> m_dialect;
};

}
