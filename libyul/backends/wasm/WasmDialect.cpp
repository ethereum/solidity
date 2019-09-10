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

#include <libyul/backends/wasm/WasmDialect.h>

using namespace std;
using namespace yul;

WasmDialect::WasmDialect():
	Dialect{AsmFlavour::Strict}
{
	for (auto const& name: {
		"i64.add",
		"i64.sub",
		"i64.mul",
		"i64.div_u",
		"i64.rem_u",
		"i64.and",
		"i64.or",
		"i64.xor",
		"i64.shl",
		"i64.shr_u",
		"i64.eq",
		"i64.ne",
		"i64.lt_u",
		"i64.gt_u",
		"i64.le_u",
		"i64.ge_u"
	})
		addFunction(name, 2, 1);

	addFunction("i64.eqz", 1, 1);
	addFunction("i64.store", 2, 0);
	addFunction("i64.load", 1, 1);

	addFunction("drop", 1, 0);
	addFunction("unreachable", 0, 0);
}

BuiltinFunction const* WasmDialect::builtin(YulString _name) const
{
	auto it = m_functions.find(_name);
	if (it != m_functions.end())
		return &it->second;
	else
		return nullptr;
}

WasmDialect const& WasmDialect::instance()
{
	static std::unique_ptr<WasmDialect> dialect;
	static YulStringRepository::ResetCallback callback{[&] { dialect.reset(); }};
	if (!dialect)
		dialect = make_unique<WasmDialect>();
	return *dialect;
}

void WasmDialect::addFunction(string _name, size_t _params, size_t _returns)
{
	YulString name{move(_name)};
	BuiltinFunction& f = m_functions[name];
	f.name = name;
	f.parameters.resize(_params);
	f.returns.resize(_returns);
	f.movable = false;
	f.sideEffectFree = false;
	f.sideEffectFreeIfNoMSize = false;
	f.isMSize = false;
	f.literalArguments = false;
}
