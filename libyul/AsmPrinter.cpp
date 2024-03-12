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

#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/Exceptions.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <range/v3/view/transform.hpp>

#include <functional>
#include <memory>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::util;
using namespace solidity::yul;

std::string AsmPrinter::operator()(Literal const& _literal)
{
	yulAssert(validLiteral(_literal));

	std::string const locationComment = formatDebugData(_literal);
	std::string const formattedValue = formatLiteral(_literal);
	if ( _literal.debugData)
		m_currentDebugAttributes = _literal.debugData->attributes;
	switch (_literal.kind)
	{
	case LiteralKind::Number:
		return locationComment + formattedValue + appendTypeName(_literal.type);
	case LiteralKind::Boolean:
		return locationComment + formattedValue + appendTypeName(_literal.type, true);
	case LiteralKind::String:
		break;
	}

	return locationComment + escapeAndQuoteString(formattedValue) + appendTypeName(_literal.type);
}

std::string AsmPrinter::operator()(Identifier const& _identifier)
{
	yulAssert(!_identifier.name.empty(), "Invalid identifier.");
	if (_identifier.debugData)
		m_currentDebugAttributes = _identifier.debugData->attributes;
	return formatDebugData(_identifier) + _identifier.name.str();
}

std::string AsmPrinter::operator()(ExpressionStatement const& _statement)
{
	std::string const locationComment = formatDebugData(_statement);
	if (_statement.debugData)
		m_currentDebugAttributes = _statement.debugData->attributes;
	return locationComment + std::visit(*this, _statement.expression);
}

std::string AsmPrinter::operator()(Assignment const& _assignment)
{
	std::string const locationComment = formatDebugData(_assignment);
	if (_assignment.debugData)
		m_currentDebugAttributes = _assignment.debugData->attributes;

	yulAssert(_assignment.variableNames.size() >= 1, "");
	std::string variables = (*this)(_assignment.variableNames.front());
	for (size_t i = 1; i < _assignment.variableNames.size(); ++i)
		variables += ", " + (*this)(_assignment.variableNames[i]);

	return locationComment + variables + " := " + std::visit(*this, *_assignment.value);
}

std::string AsmPrinter::operator()(VariableDeclaration const& _variableDeclaration)
{
	std::string out = formatDebugData(_variableDeclaration);
	if (_variableDeclaration.debugData)
		m_currentDebugAttributes = _variableDeclaration.debugData->attributes;
	out += "let ";
	out += boost::algorithm::join(
		_variableDeclaration.variables | ranges::views::transform(
			[this](TypedName argument) { return formatTypedName(argument); }
		),
		", "
	);
	if (_variableDeclaration.value)
	{
		out += " := ";
		out += std::visit(*this, *_variableDeclaration.value);
	}
	return out;
}

std::string AsmPrinter::operator()(FunctionDefinition const& _functionDefinition)
{
	yulAssert(!_functionDefinition.name.empty(), "Invalid function name.");
	if (_functionDefinition.debugData)
		m_currentDebugAttributes = _functionDefinition.debugData->attributes;
	std::string out = formatDebugData(_functionDefinition);
	out += "function " + _functionDefinition.name.str() + "(";
	out += boost::algorithm::join(
		_functionDefinition.parameters | ranges::views::transform(
			[this](TypedName argument) { return formatTypedName(argument); }
		),
		", "
	);
	out += ")";
	if (!_functionDefinition.returnVariables.empty())
	{
		out += " -> ";
		out += boost::algorithm::join(
			_functionDefinition.returnVariables | ranges::views::transform(
				[this](TypedName argument) { return formatTypedName(argument); }
			),
			", "
		);
	}

	return out + "\n" + (*this)(_functionDefinition.body);
}

std::string AsmPrinter::operator()(FunctionCall const& _functionCall)
{
	std::string const locationComment = formatDebugData(_functionCall);
	std::string const functionName = (*this)(_functionCall.functionName);
	if (_functionCall.debugData)
		m_currentDebugAttributes = _functionCall.debugData->attributes;
	return
		locationComment +
		functionName + "(" +
		boost::algorithm::join(
			_functionCall.arguments | ranges::views::transform([&](auto&& _node) { return std::visit(*this, _node); }),
			", " ) +
		")";
}

std::string AsmPrinter::operator()(If const& _if)
{
	yulAssert(_if.condition, "Invalid if condition.");
	if (_if.debugData)
		m_currentDebugAttributes = _if.debugData->attributes;
	std::string out = formatDebugData(_if);
	out += "if " + std::visit(*this, *_if.condition);

	std::string body = (*this)(_if.body);
	char delim = '\n';
	if (body.find('\n') == std::string::npos)
		delim = ' ';

	return out + delim + body;
}

std::string AsmPrinter::operator()(Switch const& _switch)
{
	yulAssert(_switch.expression, "Invalid expression pointer.");
	if (_switch.debugData)
		m_currentDebugAttributes = _switch.debugData->attributes;
	std::string out = formatDebugData(_switch);
	out += "switch " + std::visit(*this, *_switch.expression);

	for (auto const& _case: _switch.cases)
	{
		if (!_case.value)
			out += "\ndefault ";
		else
			out += "\ncase " + (*this)(*_case.value) + " ";
		out += (*this)(_case.body);
	}
	return out;
}

std::string AsmPrinter::operator()(ForLoop const& _forLoop)
{
	yulAssert(_forLoop.condition, "Invalid for loop condition.");
	if (_forLoop.debugData)
		m_currentDebugAttributes = _forLoop.debugData->attributes;
	std::string const locationComment = formatDebugData(_forLoop);

	std::string pre = (*this)(_forLoop.pre);
	std::string condition = std::visit(*this, *_forLoop.condition);
	std::string post = (*this)(_forLoop.post);

	char delim = '\n';
	if (
		pre.size() + condition.size() + post.size() < 60 &&
		pre.find('\n') == std::string::npos &&
		post.find('\n') == std::string::npos
	)
		delim = ' ';
	return
		locationComment +
		("for " + std::move(pre) + delim + std::move(condition) + delim + std::move(post) + "\n") +
		(*this)(_forLoop.body);
}

std::string AsmPrinter::operator()(Break const& _break)
{
	if (_break.debugData)
		m_currentDebugAttributes = _break.debugData->attributes;
	return formatDebugData(_break) + "break";
}

std::string AsmPrinter::operator()(Continue const& _continue)
{
	if (_continue.debugData)
		m_currentDebugAttributes = _continue.debugData->attributes;
	return formatDebugData(_continue) + "continue";
}

// '_leave' and '__leave' is reserved in VisualStudio
std::string AsmPrinter::operator()(Leave const& leave_)
{
	if (leave_.debugData)
		m_currentDebugAttributes = leave_.debugData->attributes;
	return formatDebugData(leave_) + "leave";
}

std::string AsmPrinter::operator()(Block const& _block)
{
	std::string const locationComment = formatDebugData(_block);
	if (_block.debugData)
		m_currentDebugAttributes = _block.debugData->attributes;
	if (_block.statements.empty())
		return locationComment + "{ }";
	std::string body = boost::algorithm::join(
		_block.statements | ranges::views::transform([&](auto&& _node) { return std::visit(*this, _node); }),
		"\n"
	);
	if (body.size() < 30 && body.find('\n') == std::string::npos)
		return locationComment + "{ " + body + " }";
	else
	{
		boost::replace_all(body, "\n", "\n    ");
		return locationComment + "{\n    " + body + "\n}";
	}
}

std::string AsmPrinter::formatTypedName(TypedName _variable)
{
	yulAssert(!_variable.name.empty(), "Invalid variable name.");
	if (_variable.debugData)
		m_currentDebugAttributes = _variable.debugData->attributes;
	return formatDebugData(_variable) + _variable.name.str() + appendTypeName(_variable.type);
}

std::string AsmPrinter::appendTypeName(YulString _type, bool _isBoolLiteral) const
{
	if (m_dialect && !_type.empty())
	{
		if (!_isBoolLiteral && _type == m_dialect->defaultType)
			_type = {};
		else if (_isBoolLiteral && _type == m_dialect->boolType && !m_dialect->defaultType.empty())
			// Special case: If we have a bool type but empty default type, do not remove the type.
			_type = {};
	}
	if (_type.empty())
		return {};
	else
		return ":" + _type.str();
}

std::string AsmPrinter::formatSourceLocation(
	SourceLocation const& _location,
	std::map<std::string, unsigned> const& _nameToSourceIndex,
	DebugInfoSelection const& _debugInfoSelection,
	CharStreamProvider const* _soliditySourceProvider
)
{
	yulAssert(!_nameToSourceIndex.empty(), "");
	if (_debugInfoSelection.snippet)
		yulAssert(_debugInfoSelection.location, "@src tag must always contain the source location");

	if (_debugInfoSelection.none())
		return "";

	std::string sourceIndex = "-1";
	std::string solidityCodeSnippet = "";
	if (_location.sourceName)
	{
		sourceIndex = std::to_string(_nameToSourceIndex.at(*_location.sourceName));

		if (
			_debugInfoSelection.snippet &&
			_soliditySourceProvider &&
			!_soliditySourceProvider->charStream(*_location.sourceName).isImportedFromAST()
		)
		{
			solidityCodeSnippet = escapeAndQuoteString(
				_soliditySourceProvider->charStream(*_location.sourceName).singleLineSnippet(_location)
			);

			// On top of escaping quotes we also escape the slash inside any `*/` to guard against
			// it prematurely terminating multi-line comment blocks. We do not escape all slashes
			// because the ones without `*` are not dangerous and ignoring them reduces visual noise.
			boost::replace_all(solidityCodeSnippet, "*/", "*\\/");
		}
	}

	std::string sourceLocation =
		"@src " +
		sourceIndex +
		":" +
		std::to_string(_location.start) +
		":" +
		std::to_string(_location.end);

	return sourceLocation + (solidityCodeSnippet.empty() ? "" : "  ") + solidityCodeSnippet;
}

std::string AsmPrinter::formatDebugData(langutil::DebugData::ConstPtr const& _debugData, bool _statement)
{
	if (!_debugData || m_debugInfoSelection.none())
		return "";

	std::vector<std::string> items;
	if (auto id = _debugData->astID)
		if (m_debugInfoSelection.astID)
			items.emplace_back("@ast-id " + std::to_string(*id));

	if (
		m_lastLocation != _debugData->originLocation &&
		!m_nameToSourceIndex.empty()
	)
	{
		m_lastLocation = _debugData->originLocation;

		items.emplace_back(formatSourceLocation(
			_debugData->originLocation,
			m_nameToSourceIndex,
			m_debugInfoSelection,
			m_soliditySourceProvider
		));
	}
	// if (m_currentDebugAttributes != _debugData->attributes)
	// {
		// Json diff = Json::diff(m_currentDebugAttributes, _debugData->attributes);
		// if (!diff.empty())
			// items.emplace_back("@debug.patch " + diff.dump());
	// }
	if (_debugData->attributes.has_value() && m_currentDebugAttributes != *_debugData->attributes)
	{
		Json attributes = Json::array();
		for (auto const& attribute:*_debugData->attributes)
			if ((*attribute != Json::object()) && (*attribute != Json::array()))
				attributes.emplace_back(*attribute);
		if (!attributes.empty())
			items.emplace_back("@debug.set " + (attributes.size() == 1 ? attributes.at(0).dump() : attributes.dump()));
	}
	std::string commentBody = joinHumanReadable(items, " ");
	if (commentBody.empty())
		return "";
	else
		return
			_statement ?
			"/// " + commentBody + "\n" :
			"/** " + commentBody + " */ ";
}
