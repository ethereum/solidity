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
 * @date 2019
 * @author julius <djudju@protonmail.com>
 * Converts inline assembly AST to JSON format
 */

#pragma once

#include <libyul/ASTForward.h>
#include <liblangutil/SourceLocation.h>
#include <libsolutil/JSON.h>
#include <boost/variant/static_visitor.hpp>
#include <optional>
#include <vector>

namespace solidity::yul
{

/**
 * Converter of the yul AST into JSON format
 */
class AsmJsonConverter: public boost::static_visitor<Json>
{
public:
	/// Create a converter to JSON for any block of inline assembly
	/// @a _sourceIndex to be used to abbreviate source name in the source locations
	explicit AsmJsonConverter(std::optional<size_t> _sourceIndex): m_sourceIndex(_sourceIndex) {}

	Json operator()(Block const& _node) const;
	Json operator()(NameWithDebugData const& _node) const;
	Json operator()(Literal const& _node) const;
	Json operator()(Identifier const& _node) const;
	Json operator()(Assignment const& _node) const;
	Json operator()(VariableDeclaration const& _node) const;
	Json operator()(FunctionDefinition const& _node) const;
	Json operator()(FunctionCall const& _node) const;
	Json operator()(If const& _node) const;
	Json operator()(Switch const& _node) const;
	Json operator()(Case const& _node) const;
	Json operator()(ForLoop const& _node) const;
	Json operator()(Break const& _node) const;
	Json operator()(Continue const& _node) const;
	Json operator()(Leave const& _node) const;
	Json operator()(ExpressionStatement const& _node) const;
	Json operator()(Label const& _node) const;

private:
	Json createAstNode(langutil::SourceLocation const& _originLocation, langutil::SourceLocation const& _nativeLocation, std::string _nodeType) const;
	template <class T>
	Json vectorOfVariantsToJson(std::vector<T> const& vec) const;

	std::optional<size_t> const m_sourceIndex;
};

}
