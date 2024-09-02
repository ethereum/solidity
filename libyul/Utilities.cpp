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
 * Some useful snippets for the optimiser.
 */

#include <libyul/Utilities.h>

#include <libyul/AST.h>
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>
#include <libsolutil/FixedHash.h>

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <sstream>
#include <vector>

using namespace solidity;
using namespace solidity::yul;
using namespace solidity::util;

std::string solidity::yul::reindent(std::string const& _code)
{
	int constexpr indentationWidth = 4;

	auto constexpr static countBraces = [](std::string const& _s) noexcept -> int
	{
		auto const i = _s.find("//");
		auto const e = i == _s.npos ? end(_s) : next(begin(_s), static_cast<ptrdiff_t>(i));
		auto const opening = count_if(begin(_s), e, [](auto ch) { return ch == '{' || ch == '('; });
		auto const closing = count_if(begin(_s), e, [](auto ch) { return ch == '}' || ch == ')'; });
		return int(opening - closing);
	};

	std::vector<std::string> lines;
	boost::split(lines, _code, boost::is_any_of("\n"));
	for (std::string& line: lines)
		boost::trim(line);

	// Reduce multiple consecutive empty lines.
	lines = fold(lines, std::vector<std::string>{}, [](auto&& _lines, auto&& _line) {
		if (!(_line.empty() && !_lines.empty() && _lines.back().empty()))
			_lines.emplace_back(std::move(_line));
		return std::move(_lines);
	});

	std::stringstream out;
	int depth = 0;

	for (std::string const& line: lines)
	{
		int const diff = countBraces(line);
		if (diff < 0)
			depth += diff;

		if (!line.empty())
		{
			for (int i = 0; i < depth * indentationWidth; ++i)
				out << ' ';
			out << line;
		}
		out << '\n';

		if (diff > 0)
			depth += diff;
	}

	return out.str();
}

LiteralValue solidity::yul::valueOfNumberLiteral(std::string_view const _literal)
{
	return LiteralValue{LiteralValue::Data(_literal), std::string(_literal)};
}

LiteralValue solidity::yul::valueOfStringLiteral(std::string_view const _literal)
{
	std::string const s(_literal);
	return LiteralValue{u256(h256(s, h256::FromBinary, h256::AlignLeft)), s};
}

LiteralValue solidity::yul::valueOfBuiltinStringLiteralArgument(std::string_view _literal)
{
	return LiteralValue{std::string(_literal)};
}

LiteralValue solidity::yul::valueOfBoolLiteral(std::string_view const _literal)
{
	if (_literal == "true")
		return LiteralValue{true};
	else if (_literal == "false")
		return LiteralValue{false};

	yulAssert(false, "Unexpected bool literal value!");
}

LiteralValue solidity::yul::valueOfLiteral(std::string_view const _literal, LiteralKind const& _kind, bool const _unlimitedLiteralArgument)
{
	switch (_kind)
	{
	case LiteralKind::Number:
		return valueOfNumberLiteral(_literal);
	case LiteralKind::Boolean:
		return valueOfBoolLiteral(_literal);
	case LiteralKind::String:
		return _unlimitedLiteralArgument ? valueOfBuiltinStringLiteralArgument(_literal) : valueOfStringLiteral(_literal);
	}
	util::unreachable();
}

std::string solidity::yul::formatLiteral(solidity::yul::Literal const& _literal, bool const _validated)
{
	if (_validated)
		yulAssert(validLiteral(_literal), "Encountered invalid literal in formatLiteral.");

	if (_literal.value.unlimited())
		return _literal.value.builtinStringLiteralValue();

	if (_literal.value.hint())
		return *_literal.value.hint();

	if (_literal.kind == LiteralKind::Boolean)
		return _literal.value.value() == false ? "false" : "true";

	// if there is no hint and it is not a boolean, just stringify the u256 word
	return _literal.value.value().str();
}

bool solidity::yul::validLiteral(solidity::yul::Literal const& _literal)
{
	switch (_literal.kind)
	{
	case LiteralKind::Number:
		return validNumberLiteral(_literal);
	case LiteralKind::Boolean:
		return validBoolLiteral(_literal);
	case LiteralKind::String:
		return validStringLiteral(_literal);
	}
	util::unreachable();
}

bool solidity::yul::validStringLiteral(solidity::yul::Literal const& _literal)
{
	if (_literal.kind != LiteralKind::String)
		return false;

	if (_literal.value.unlimited())
		return true;

	if (_literal.value.hint())
		return _literal.value.hint()->size() <= 32 && _literal.value.value() == valueOfLiteral(*_literal.value.hint(), _literal.kind).value();

	return true;
}

bool solidity::yul::validNumberLiteral(solidity::yul::Literal const& _literal)
{
	if (_literal.kind != LiteralKind::Number || _literal.value.unlimited())
		return false;

	if (!_literal.value.hint())
		return true;

	auto const& repr = *_literal.value.hint();

	if (!isValidDecimal(repr) && !isValidHex(repr))
		return false;

	if (bigint(repr) > u256(-1))
		return false;

	if (_literal.value.value() != valueOfLiteral(repr, _literal.kind).value())
		return false;

	return true;
}

bool solidity::yul::validBoolLiteral(solidity::yul::Literal const& _literal)
{
	if (_literal.kind != LiteralKind::Boolean || _literal.value.unlimited())
		return false;

	if (_literal.value.hint() && !(*_literal.value.hint() == "true" || *_literal.value.hint() == "false"))
		return false;

	yulAssert(u256(0) == u256(false));
	yulAssert(u256(1) == u256(true));

	if (_literal.value.hint())
	{
		if (*_literal.value.hint() == "false")
			return _literal.value.value() == false;
		else
			return _literal.value.value() == true;
	}

	return _literal.value.value() == true || _literal.value.value() == false;
}

template<>
bool Less<Literal>::operator()(Literal const& _lhs, Literal const& _rhs) const
{
	if (_lhs.kind != _rhs.kind)
		return _lhs.kind < _rhs.kind;

	if (_lhs.value.unlimited() && _rhs.value.unlimited())
		yulAssert(
			_lhs.kind == LiteralKind::String && _rhs.kind == LiteralKind::String,
			"Cannot have unlimited value that is not of String kind."
		);

	return _lhs.value < _rhs.value;

}

bool SwitchCaseCompareByLiteralValue::operator()(Case const* _lhs, Case const* _rhs) const
{
	yulAssert(_lhs && _rhs, "");
	return Less<Literal*>{}(_lhs->value.get(), _rhs->value.get());
}
