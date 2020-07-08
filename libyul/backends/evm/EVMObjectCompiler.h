// SPDX-License-Identifier: GPL-3.0
/**
 * Compiler that transforms Yul Objects to EVM bytecode objects.
 */

#pragma once

namespace solidity::yul
{
struct Object;
class AbstractAssembly;
struct EVMDialect;

class EVMObjectCompiler
{
public:
	static void compile(Object& _object, AbstractAssembly& _assembly, EVMDialect const& _dialect, bool _evm15, bool _optimize);
private:
	EVMObjectCompiler(AbstractAssembly& _assembly, EVMDialect const& _dialect, bool _evm15):
		m_assembly(_assembly), m_dialect(_dialect), m_evm15(_evm15)
	{}

	void run(Object& _object, bool _optimize);

	AbstractAssembly& m_assembly;
	EVMDialect const& m_dialect;
	bool m_evm15 = false;
};

}
