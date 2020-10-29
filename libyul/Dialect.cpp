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
 * Yul dialect.
 */

#include <libyul/Dialect.h>
#include <libyul/AST.h>

using namespace solidity::yul;
using namespace std;
using namespace solidity::langutil;

Literal Dialect::zeroLiteralForType(solidity::yul::YulString _type) const
{
	if (_type == boolType && _type != defaultType)
		return {SourceLocation{}, LiteralKind::Boolean, "false"_yulstring, _type};
	return {SourceLocation{}, LiteralKind::Number, "0"_yulstring, _type};
}


Literal Dialect::trueLiteral() const
{
	if (boolType != defaultType)
		return {SourceLocation{}, LiteralKind::Boolean, "true"_yulstring, boolType};
	else
		return {SourceLocation{}, LiteralKind::Number, "1"_yulstring, defaultType};
}

bool Dialect::validTypeForLiteral(LiteralKind _kind, YulString, YulString _type) const
{
	if (_kind == LiteralKind::Boolean)
		return _type == boolType;
	else
		return true;
}

Dialect const& Dialect::yulDeprecated()
{
	static unique_ptr<Dialect> dialect;
	static YulStringRepository::ResetCallback callback{[&] { dialect.reset(); }};

	if (!dialect)
	{
		// TODO will probably change, especially the list of types.
		dialect = make_unique<Dialect>();
		dialect->defaultType = "u256"_yulstring;
		dialect->boolType = "bool"_yulstring;
		dialect->types = {
			"bool"_yulstring,
			"u8"_yulstring,
			"s8"_yulstring,
			"u32"_yulstring,
			"s32"_yulstring,
			"u64"_yulstring,
			"s64"_yulstring,
			"u128"_yulstring,
			"s128"_yulstring,
			"u256"_yulstring,
			"s256"_yulstring
		};
	};

	return *dialect;
}
