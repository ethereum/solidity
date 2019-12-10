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
 * @date 2019
 * @author julius <djudju@protonmail.com>
 * Converts inline assembly AST to JSON format
 */

#include <libyul/AsmDataForward.h>
#include <liblangutil/SourceLocation.h>
#include <json/json.h>
#include <boost/variant.hpp>
#include <vector>

namespace yul
{

/**
 * Converter of the yul AST into JSON format
 */
class AsmJsonConverter: public boost::static_visitor<Json::Value>
{
public:
	/// Create a converter to JSON for any block of inline assembly
	/// @a _sourceIndex to be used to abbreviate source name in the source locations
	explicit AsmJsonConverter(size_t _sourceIndex): m_sourceIndex(std::to_string(_sourceIndex)) {}

	Json::Value operator()(Block const& _node) const;
	Json::Value operator()(TypedName const& _node) const;
	Json::Value operator()(Literal const& _node) const;
	Json::Value operator()(Identifier const& _node) const;
	Json::Value operator()(Assignment const& _node) const;
	Json::Value operator()(VariableDeclaration const& _node) const;
	Json::Value operator()(FunctionDefinition const& _node) const;
	Json::Value operator()(FunctionCall const& _node) const;
	Json::Value operator()(If const& _node) const;
	Json::Value operator()(Switch const& _node) const;
	Json::Value operator()(Case const& _node) const;
	Json::Value operator()(ForLoop const& _node) const;
	Json::Value operator()(Break const& _node) const;
	Json::Value operator()(Continue const& _node) const;
	Json::Value operator()(Leave const& _node) const;
	Json::Value operator()(ExpressionStatement const& _node) const;
	Json::Value operator()(Label const& _node) const;

private:
	Json::Value createAstNode(langutil::SourceLocation const& _location, std::string _nodeType) const;
	template <class T>
	Json::Value vectorOfVariantsToJson(std::vector<T> const& vec) const;

	std::string const m_sourceIndex;
};

}
