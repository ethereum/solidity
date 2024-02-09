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
 * @author Alexander Arlt <alexander.arlt@arlt-labs.com>
 * @date 2024
 * Translate an VariableDeclaration AST node into its ethdebug json representation.
 */

#pragma once

#include <libethdebug/translator/detail/PrepareTranslator.h>

#include <utility>

namespace ethdebug
{

template<>
Json::Value toJson(solidity::frontend::Type const* _type)
{
	Json::Value result = Json::objectValue;
	switch (_type->category())
	{
	case solidity::frontend::Type::Category::Address:
		result["kind"] = "address";
		break;
	case solidity::frontend::Type::Category::Integer:
	{
		auto integer = dynamic_cast<solidity::frontend::IntegerType const*>(_type);
		if (integer->isSigned())
			result["kind"] = "int";
		else
			result["kind"] = "uint";
		result["bits"] = integer->numBits();
		break;
	}
	case solidity::frontend::Type::Category::RationalNumber:
		result["kind"] = "rational";
		break;
	case solidity::frontend::Type::Category::StringLiteral:
		result["kind"] = "string";
		break;
	case solidity::frontend::Type::Category::Bool:
		result["kind"] = "bool";
		break;
	case solidity::frontend::Type::Category::FixedPoint:
		result["kind"] = "fixed";
		break;
	case solidity::frontend::Type::Category::Array:
		result["kind"] = "array";
		break;
	case solidity::frontend::Type::Category::ArraySlice:
		result["kind"] = "slice";
		break;
	case solidity::frontend::Type::Category::FixedBytes:
		result["kind"] = "bytes";
		break;
	case solidity::frontend::Type::Category::Contract:
		result["kind"] = "contract";
		break;
	case solidity::frontend::Type::Category::Struct:
		result["kind"] = "struct";
		break;
	case solidity::frontend::Type::Category::Function:
		result["kind"] = "function";
		break;
	case solidity::frontend::Type::Category::Enum:
		result["kind"] = "enum";
		break;
	case solidity::frontend::Type::Category::UserDefinedValueType:
		result["kind"] = "udvt";
		break;
	case solidity::frontend::Type::Category::Tuple:
		result["kind"] = "tuple";
		break;
	case solidity::frontend::Type::Category::Mapping:
		result["kind"] = "mapping";
		break;
	case solidity::frontend::Type::Category::TypeType:
		result["kind"] = "type-type";
		break;
	case solidity::frontend::Type::Category::Modifier:
		result["kind"] = "modifier";
		break;
	case solidity::frontend::Type::Category::Magic:
		result["kind"] = "magic";
		break;
	case solidity::frontend::Type::Category::Module:
		result["kind"] = "module";
		break;
	case solidity::frontend::Type::Category::InaccessibleDynamic:
		result["kind"] = "inaccessible-dynamic";
		break;
	}
	return result;
}

} // namespace ethdebug
