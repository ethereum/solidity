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
#include <libyul/YulName.h>

#include <libsolutil/CommonData.h>

#include <liblangutil/CharStreamProvider.h>
#include <liblangutil/DebugInfoSelection.h>
#include <liblangutil/DebugData.h>

#include <map>

namespace solidity::yul
{

/**
 * Converts a parsed Yul AST into readable string representation.
 * Ignores source locations.
 */
class AsmPrinter
{
public:
	explicit AsmPrinter(
		std::optional<std::map<unsigned, std::shared_ptr<std::string const>>> _sourceIndexToName = {},
		langutil::DebugInfoSelection const& _debugInfoSelection = langutil::DebugInfoSelection::Default(),
		langutil::CharStreamProvider const* _soliditySourceProvider = nullptr
	):
		m_debugInfoSelection(_debugInfoSelection),
		m_soliditySourceProvider(_soliditySourceProvider)
	{
		if (_sourceIndexToName)
			for (auto&& [index, name]: *_sourceIndexToName)
				m_nameToSourceIndex[*name] = index;
	}

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
	std::string formatNameWithDebugData(NameWithDebugData _variable);
	std::string formatDebugData(langutil::DebugData::ConstPtr const& _debugData, bool _statement);
	template <class T>
	std::string formatDebugData(T const& _node)
	{
		bool isExpression = std::is_constructible<Expression, T>::value;
		return formatDebugData(_node.debugData, !isExpression);
	}

	std::map<std::string, unsigned> m_nameToSourceIndex;
	langutil::SourceLocation m_lastLocation = {};
	langutil::DebugInfoSelection m_debugInfoSelection = {};
	langutil::CharStreamProvider const* m_soliditySourceProvider = nullptr;
};

}
