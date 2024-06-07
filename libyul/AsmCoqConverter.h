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
class AsmCoqConverter: public boost::static_visitor<Json>
{
public:
	/// Create a converter to JSON for any block of inline assembly
	/// @a _sourceIndex to be used to abbreviate source name in the source locations
	explicit AsmCoqConverter(std::optional<size_t> _sourceIndex): m_sourceIndex(_sourceIndex) {}

	std::string operator()(Block const& _node);
	std::string operator()(NameWithDebugData const& _node);
	std::string operator()(Literal const& _node);
	std::string operator()(Identifier const& _node);
	std::string operator()(Assignment const& _node);
	std::string operator()(VariableDeclaration const& _node);
	std::string operator()(FunctionDefinition const& _node);
	std::string operator()(FunctionCall const& _node);
	std::string operator()(If const& _node);
	std::string operator()(Switch const& _node);
	std::string operator()(Case const& _node);
	std::string operator()(ForLoop const& _node);
	std::string operator()(Break const& _node);
	std::string operator()(Continue const& _node);
	std::string operator()(Leave const& _node);
	std::string operator()(ExpressionStatement const& _node);
	std::string operator()(Label const& _node);

private:
	std::string indent() const;
	std::string rawLiteral(Literal const& _node) const;
	template <class T>
	std::string rawAssign(
		bool isDeclaration,
		std::vector<T> const& variables,
		std::unique_ptr<Expression> const& value
	);

	std::optional<size_t> const m_sourceIndex;
	uint64_t m_indent = 0;
};

}
