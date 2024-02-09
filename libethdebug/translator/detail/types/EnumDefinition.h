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
* Translate an EnumDefinition AST node into its ethdebug json representation.
 */

#pragma once

#include <libethdebug/translator/detail/PrepareTranslator.h>

#include <utility>

namespace ethdebug
{

template<>
Json::Value toJson(solidity::frontend::EnumDefinition const* _enum)
{
	Json::Value result = Json::objectValue;
	result["kind"] = "enum";

	Json::Value values = Json::arrayValue;
	for (auto const& _value : _enum->members())
		values.append(_value->name());
	result["values"] = values;

	return result;
}

} // namespace ethdebug
