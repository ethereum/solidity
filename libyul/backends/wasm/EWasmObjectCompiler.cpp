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
 * Compiler that transforms Yul Objects to EWasm text representation.
 */

#include <libyul/backends/wasm/EWasmObjectCompiler.h>

#include <libyul/backends/wasm/EWasmCodeTransform.h>
#include <libyul/backends/wasm/BinaryTransform.h>
#include <libyul/backends/wasm/EWasmToText.h>

#include <libyul/Object.h>
#include <libyul/Exceptions.h>

#include <libdevcore/CommonData.h>

using namespace yul;
using namespace std;

pair<string, dev::bytes> EWasmObjectCompiler::compile(Object& _object, Dialect const& _dialect)
{
	EWasmObjectCompiler compiler(_dialect);
	wasm::Module module = compiler.run(_object);
	return {EWasmToText().run(module), wasm::BinaryTransform::run(module)};
}

wasm::Module EWasmObjectCompiler::run(Object& _object)
{
	yulAssert(_object.analysisInfo, "No analysis info.");
	yulAssert(_object.code, "No code.");

	wasm::Module module = EWasmCodeTransform::run(m_dialect, *_object.code);

	for (auto& subNode: _object.subObjects)
		if (Object* subObject = dynamic_cast<Object*>(subNode.get()))
			module.subModules[subObject->name.str()] = run(*subObject);
		else
			yulAssert(false, "Data is not yet supported for EWasm.");

	return module;
}
