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
/**
 * Dialects for Wasm.
 */

#pragma once

#include <libyul/Dialect.h>

#include <map>

namespace yul
{

class YulString;
using Type = YulString;
struct FunctionCall;
struct Object;

/**
 * Yul dialect for Wasm as a backend.
 *
 * Builtin functions are a subset of the wasm instructions, always implicitly assuming
 * unsigned 64 bit types.
 *
 * !This is subject to changes!
 */
struct WasmDialect: public Dialect
{
	WasmDialect();

	BuiltinFunction const* builtin(YulString _name) const override;
	BuiltinFunction const* discardFunction() const override { return builtin("drop"_yulstring); }
	BuiltinFunction const* equalityFunction() const override { return builtin("i64.eq"_yulstring); }

	std::set<YulString> fixedFunctionNames() const override { return {"main"_yulstring}; }

	static WasmDialect const& instance();

private:
	void addFunction(std::string _name, size_t _params, size_t _returns, bool _literalArguments = false);

	std::map<YulString, BuiltinFunction> m_functions;
};

}
