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

#include <libsmtutil/SMTLib2Parser.h>

#include <liblangutil/Common.h>

#include <libsolutil/Visitor.h>
#include <libsolutil/StringUtils.h>


using namespace solidity::langutil;
using namespace solidity::smtutil;

std::string SMTLib2Expression::toString() const {
	return std::visit(solidity::util::GenericVisitor{
			[](std::string const& _sv) { return _sv; },
			[](std::vector<SMTLib2Expression> const& _subExpr) {
				std::vector<std::string> formatted;
				for (auto const& item: _subExpr)
					formatted.emplace_back(item.toString());
				return "(" + solidity::util::joinHumanReadable(formatted, " ") + ")";
			}
	}, data);
}

SMTLib2Expression SMTLib2Parser::parseExpression() {
	skipWhitespace();
	if (token() == '(')
	{
		advance();
		skipWhitespace();
		std::vector<SMTLib2Expression> subExpressions;
		while (token() != 0 && token() != ')')
		{
			subExpressions.emplace_back(parseExpression());
			skipWhitespace();
		}
		if (token() != ')')
			throw ParsingException{};
		// Simulate whitespace because we do not want to read the next token since it might block.
		m_token = ' ';
		return {std::move(subExpressions)};
	} else
		return {parseToken()};
}

std::string SMTLib2Parser::parseToken() {
	std::string result;

	skipWhitespace();
	bool isPipe = token() == '|';
	if (isPipe)
		advance();
	while (token() != 0)
	{
		char c = token();
		if (isPipe && c == '|')
		{
			advance();
			break;
		} else if (!isPipe && (isWhiteSpace(c) || c == '(' || c == ')'))
			break;
		result.push_back(c);
		advance();
	}
	return result;
}

void SMTLib2Parser::advance() {
	if (!m_input.good())
		throw ParsingException{};
	m_token = static_cast<char>(m_input.get());
	if (token() == ';')
		while (token() != '\n' && token() != 0)
			m_token = static_cast<char>(m_input.get());
}

void SMTLib2Parser::skipWhitespace() {
	while (isWhiteSpace(token()))
		advance();
}
