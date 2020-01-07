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
 * Compiler that transforms Yul Objects to Wasm text and binary representation (Ewasm flavoured).
 */

#include <libyul/backends/wasm/WasmObjectCompiler.h>

#include <libyul/backends/wasm/WasmCodeTransform.h>
#include <libyul/backends/wasm/BinaryTransform.h>
#include <libyul/backends/wasm/TextTransform.h>

#include <libyul/Object.h>
#include <libyul/Exceptions.h>

#include <libsolutil/CommonData.h>

using namespace solidity;
using namespace solidity::yul;
using namespace std;

pair<string, bytes> WasmObjectCompiler::compile(Object& _object, Dialect const& _dialect)
{
	WasmObjectCompiler compiler(_dialect);
	wasm::Module module = compiler.run(_object);
	return {wasm::TextTransform().run(module), wasm::BinaryTransform::run(module)};
}

wasm::Module WasmObjectCompiler::run(Object& _object)
{
	yulAssert(_object.analysisInfo, "No analysis info.");
	yulAssert(_object.code, "No code.");

	wasm::Module module = WasmCodeTransform::run(m_dialect, *_object.code);

	for (auto& subNode: _object.subObjects)
		if (Object* subObject = dynamic_cast<Object*>(subNode.get()))
			module.subModules[subObject->name.str()] = run(*subObject);
		else
			yulAssert(false, "Data is not yet supported for Wasm.");

	return module;
}
