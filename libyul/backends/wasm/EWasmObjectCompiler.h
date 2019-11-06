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

#pragma once

#include <string>
#include <vector>
#include <tuple>

namespace dev
{
using bytes = std::vector<uint8_t>;
}
namespace yul
{
struct Object;
struct Dialect;
namespace wasm
{
struct Module;
}

class EWasmObjectCompiler
{
public:
	/// Compiles the given object and returns the WAST and the binary representation.
	static std::pair<std::string, dev::bytes> compile(Object& _object, Dialect const& _dialect);
private:
	EWasmObjectCompiler(Dialect const& _dialect):
		m_dialect(_dialect)
	{}

	wasm::Module run(Object& _object);

	Dialect const& m_dialect;
};

}
