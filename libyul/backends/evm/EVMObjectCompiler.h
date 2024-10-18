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
 * Compiler that transforms Yul Objects to EVM bytecode objects.
 */

#pragma once

#include <optional>
#include <cstdint>

namespace solidity::yul
{
class Object;
class AbstractAssembly;
class EVMDialect;

class EVMObjectCompiler
{
public:
	static void compile(
		Object const& _object,
		AbstractAssembly& _assembly,
		EVMDialect const& _dialect,
		bool _optimize,
		std::optional<uint8_t> _eofVersion
	);
private:
	EVMObjectCompiler(AbstractAssembly& _assembly, EVMDialect const& _dialect, std::optional<uint8_t> _eofVersion):
		m_assembly(_assembly), m_dialect(_dialect), m_eofVersion(_eofVersion)
	{}

	void run(Object const& _object, bool _optimize);

	AbstractAssembly& m_assembly;
	EVMDialect const& m_dialect;
	std::optional<uint8_t> m_eofVersion;
};

}
