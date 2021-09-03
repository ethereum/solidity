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
 * @author Christian <c@ethdev.com>
 * @date 2017
 * Converts a parsed assembly into its textual form.
 */

#pragma once

#include <libyul/ASTForward.h>
#include <libyul/YulString.h>

#include <libsolutil/CommonData.h>

#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/SourceLocation.h>

#include <map>

namespace solidity::yul
{
struct Dialect;

/**
 * Converts a parsed Yul AST into readable string representation.
 * Ignores source locations.
 * If a dialect is provided, the dialect's default type is omitted.
 */
class AsmPrinter
{
public:
	explicit AsmPrinter(
		Dialect const* _dialect = nullptr,
		std::optional<std::map<unsigned, std::shared_ptr<std::string const>>> _sourceIndexToName = {},
		langutil::CharStreamProvider const* _soliditySourceProvider = nullptr
	):
		m_dialect(_dialect),
		m_soliditySourceProvider(_soliditySourceProvider)
	{
		if (_sourceIndexToName)
			for (auto&& [index, name]: *_sourceIndexToName)
				m_nameToSourceIndex[*name] = index;
	}


	explicit AsmPrinter(
		Dialect const& _dialect,
		std::optional<std::map<unsigned, std::shared_ptr<std::string const>>> _sourceIndexToName = {},
		langutil::CharStreamProvider const* _soliditySourceProvider = nullptr
	): AsmPrinter(&_dialect, _sourceIndexToName, _soliditySourceProvider) {}

	std::string operator()(Literal const& _literal);
	std::string operator()(Identifier const& _identifier);
	std::string operator()(ExpressionStatement const& _expr);
	std::string operator()(Assignment const& _assignment);
	std::string operator()(VariableDeclaration const& _variableDeclaration);
	std::string operator()(FunctionDefinition const& _functionDefinition);
	std::string operator()(FunctionCall const& _functionCall);
	std::string operator()(If const& _if);
	std::string operator()(Switch const& _switch);
	std::string operator()(ForLoop const& _forLoop);
	std::string operator()(Break const& _break);
	std::string operator()(Continue const& _continue);
	std::string operator()(Leave const& _continue);
	std::string operator()(Block const& _block);

	static std::string formatSourceLocation(
		langutil::SourceLocation const& _location,
		std::map<std::string, unsigned> const& _nameToSourceIndex,
		langutil::DebugInfoSelection const& _debugInfoSelection = langutil::DebugInfoSelection::Default(),
		langutil::CharStreamProvider const* m_soliditySourceProvider = nullptr
	);

private:
	std::string formatTypedName(TypedName _variable);
	std::string appendTypeName(YulString _type, bool _isBoolLiteral = false) const;
	std::string formatDebugData(std::shared_ptr<DebugData const> const& _debugData, bool _statement);
	template <class T>
	std::string formatDebugData(T const& _node)
	{
		bool isExpression = std::is_constructible<Expression, T>::value;
		return formatDebugData(_node.debugData, !isExpression);
	}

	Dialect const* const m_dialect = nullptr;
	std::map<std::string, unsigned> m_nameToSourceIndex;
	langutil::SourceLocation m_lastLocation = {};
	langutil::CharStreamProvider const* m_soliditySourceProvider = nullptr;
};

}
