// SPDX-License-Identifier: GPL-3.0
/**
 * Compiler that transforms Yul Objects to Wasm text and binary representation (Ewasm flavoured).
 */

#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <libsolutil/Common.h>     // solidity::bytes

namespace solidity::yul
{
struct Object;
struct Dialect;
namespace wasm
{
struct Module;
}

class WasmObjectCompiler
{
public:
	/// Compiles the given object and returns the Wasm text and binary representation.
	static std::pair<std::string, bytes> compile(Object& _object, Dialect const& _dialect);
private:
	WasmObjectCompiler(Dialect const& _dialect):
		m_dialect(_dialect)
	{}

	wasm::Module run(Object& _object);

	Dialect const& m_dialect;
};

}
