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
 * Compiler that transforms Yul Objects to EVM bytecode objects.
 */


namespace yul
{
struct Object;
class AbstractAssembly;

class EVMObjectCompiler
{
public:
	static void compile(Object& _object, AbstractAssembly& _assembly, bool _yul, bool _evm15);
private:
	EVMObjectCompiler(AbstractAssembly& _assembly, bool _yul, bool _evm15):
		m_assembly(_assembly), m_yul(_yul), m_evm15(_evm15)
	{}

	void run(Object& _object);

	AbstractAssembly& m_assembly;
	bool m_yul = false;
	bool m_evm15 = false;
};

}
