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
 * Dialects for Wasm.
 */

#pragma once

#include <libyul/Dialect.h>

#include <map>

namespace solidity::yul
{

class YulString;
using Type = YulString;
struct FunctionCall;
struct Object;

/**
 * Yul dialect for Wasm as a backend.
 *
 * Builtin functions are a subset of the wasm instructions.
 *
 * There is a builtin function `i32.drop` that takes an i32, while `drop` takes i64.
 *
 */
struct WasmDialect: public Dialect
{
	WasmDialect();

	BuiltinFunction const* builtin(YulString _name) const override;
	BuiltinFunction const* discardFunction(YulString _type) const override;
	BuiltinFunction const* equalityFunction(YulString _type) const override;
	BuiltinFunction const* booleanNegationFunction() const override { return builtin("i32.eqz"_yulstring); }

	std::set<YulString> fixedFunctionNames() const override { return {"main"_yulstring}; }

	static WasmDialect const& instance();

private:
	void addEthereumExternals();

	void addFunction(
		std::string _name,
		std::vector<YulString> _params,
		std::vector<YulString> _returns,
		bool _movable = true,
		std::vector<bool> _literalArguments = std::vector<bool>{}
	);

	std::map<YulString, BuiltinFunction> m_functions;
};

}
