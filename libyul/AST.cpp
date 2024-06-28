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

#include <libyul/AST.h>
#include <libyul/Exceptions.h>

namespace solidity::yul
{

LiteralValue::LiteralValue(std::string _builtinStringLiteralValue):
	m_numericValue(std::nullopt),
	m_stringValue(std::make_shared<std::string>(std::move(_builtinStringLiteralValue)))
{ }

LiteralValue::LiteralValue(solidity::yul::LiteralValue::Data const& _data, std::optional<std::string> const& _hint):
	m_numericValue(_data),
	m_stringValue(_hint ? std::make_shared<std::string>(*_hint) : nullptr)
{ }

LiteralValue::Data const& LiteralValue::value() const
{
	yulAssert(!unlimited());
	return *m_numericValue;
}

LiteralValue::BuiltinStringLiteralData const& LiteralValue::builtinStringLiteralValue() const
{
	yulAssert(unlimited());
	return *m_stringValue;
}

bool LiteralValue::unlimited() const
{
	return !m_numericValue.has_value();
}

LiteralValue::RepresentationHint const& LiteralValue::hint() const
{
	yulAssert(!unlimited());
	return m_stringValue;
}

bool LiteralValue::operator==(LiteralValue const& _rhs) const
{
	if (unlimited() != _rhs.unlimited())
		return false;

	if (unlimited())
		return builtinStringLiteralValue() == _rhs.builtinStringLiteralValue();

	return value() == _rhs.value();
}

bool LiteralValue::operator<(solidity::yul::LiteralValue const& _rhs) const
{
	if (unlimited() != _rhs.unlimited())
		return unlimited() < _rhs.unlimited();

	if (unlimited())
		return builtinStringLiteralValue() < _rhs.builtinStringLiteralValue();

	return value() < _rhs.value();
}

}
