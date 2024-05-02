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

#pragma once

#include <libsmtutil/Exceptions.h>

#include <iostream>
#include <string>
#include <variant>
#include <vector>

/**
 * The following is a parser for SMT-LIB2 expressions developed originally by @chriseth as part of solsmt.
 */

namespace solidity::smtutil
{

struct SMTLib2Expression {
	using args_t = std::vector<SMTLib2Expression>;
	std::variant<std::string, args_t> data;

	[[nodiscard]] std::string toString() const;
};

inline bool isAtom(SMTLib2Expression const& expr)
{
	return std::holds_alternative<std::string>(expr.data);
}

inline std::string const& asAtom(SMTLib2Expression const& expr)
{
	smtAssert(isAtom(expr));
	return std::get<std::string>(expr.data);
}

inline auto const& asSubExpressions(SMTLib2Expression const& expr)
{
	smtAssert(!isAtom(expr));
	return std::get<SMTLib2Expression::args_t>(expr.data);
}

inline auto& asSubExpressions(SMTLib2Expression& expr)
{
	smtAssert(!isAtom(expr));
	return std::get<SMTLib2Expression::args_t>(expr.data);
}

class SMTLib2Parser {
public:
	class ParsingException {};

	explicit SMTLib2Parser(std::istream& _input) :
			m_input(_input),
			m_token(static_cast<char>(m_input.get())) {}

	SMTLib2Expression parseExpression();

	bool isEOF()
	{
		skipWhitespace();
		return m_input.eof();
	}

private:
	std::string parseToken();

	void skipWhitespace();

	[[nodiscard]] char token() const
	{
		return m_token;
	}

	void advance();

	std::istream& m_input;
	char m_token = 0;
};
}
