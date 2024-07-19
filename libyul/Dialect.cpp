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
using namespace solidity::langutil;

Literal Dialect::zeroLiteralForType(YulName const _type, YulNameRepository const& _nameRepository) const
{
	if (_type == _nameRepository.predefined().boolType && _type != _nameRepository.predefined().defaultType)
		return {DebugData::create(), LiteralKind::Boolean, LiteralValue(false), _type};
	return {DebugData::create(), LiteralKind::Number, LiteralValue(0, std::nullopt), _type};
}

bool Dialect::validTypeForLiteral(LiteralKind _kind, LiteralValue const&, std::string_view _type) const
{
	if (_kind == LiteralKind::Boolean)
		return _type == boolType;
	else
		return true;
}

bool Dialect::validTypeForLiteral(LiteralKind _kind, LiteralValue const&, Type _type, YulNameRepository const& _nameRepository) const
{
	if (_kind == LiteralKind::Boolean)
		return _type == _nameRepository.predefined().boolType;
	else
		return true;
}

Dialect const& Dialect::yulDeprecated()
{
	static std::unique_ptr<Dialect> dialect;

	if (!dialect)
	{
		// TODO will probably change, especially the list of types.
		dialect = std::make_unique<Dialect>();
		dialect->defaultType = "u256";
		dialect->boolType = "bool";
		dialect->types = {
			"bool",
			"u8",
			"s8",
			"u32",
			"s32",
			"u64",
			"s64",
			"u128",
			"s128",
			"u256",
			"s256"
		};
	}

	return *dialect;
}
