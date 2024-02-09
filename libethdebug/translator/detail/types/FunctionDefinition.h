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
 * Translate an FunctionDefinition AST node into its ethdebug json representation.
 */

#pragma once

#include <libethdebug/translator/detail/PrepareTranslator.h>

#include <utility>

namespace ethdebug
{

template<>
Json::Value toJson(solidity::frontend::FunctionDefinition const* _functionDefinition)
{
	Json::Value result = Json::objectValue;
	if (_functionDefinition->isOrdinary())
		result["kind"] = "function";
	else if (_functionDefinition->isConstructor())
		result["kind"] = "constructor";
	else if (_functionDefinition->isFallback())
		result["kind"] = "fallback";
	else
		solAssert(false);

	if (!_functionDefinition->noVisibilitySpecified())
	{
		switch (_functionDefinition->visibility())
		{
		case solidity::frontend::Visibility::Default:
		case solidity::frontend::Visibility::Private:
		case solidity::frontend::Visibility::Internal:
			result["internal"] = true;
			break;
		case solidity::frontend::Visibility::Public:
		case solidity::frontend::Visibility::External:
			result["external"] = true;
			break;
		}
	}

	Json::Value definition = Json::objectValue;
	definition["name"] = _functionDefinition->name();
	result["definition"] = definition;

	Json::Value parameters = Json::arrayValue;
	for (auto const& param: _functionDefinition->parameters())
		parameters.append(toJson(param.get()));
	result["parameters"] = parameters;

	Json::Value returns = Json::arrayValue;
	for (auto const& param: _functionDefinition->returnParameters())
		returns.append(toJson(param.get()));
	result["returns"] = returns;

	return result;
}

} // namespace ethdebug
