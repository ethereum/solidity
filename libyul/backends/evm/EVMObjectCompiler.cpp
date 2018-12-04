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

#include <libyul/backends/evm/EVMObjectCompiler.h>

#include <libyul/backends/evm/EVMCodeTransform.h>
#include <libyul/Object.h>
#include <libyul/Exceptions.h>

using namespace yul;
using namespace std;

void EVMObjectCompiler::compile(Object& _object, AbstractAssembly& _assembly, bool _yul, bool _evm15)
{
	EVMObjectCompiler compiler(_assembly, _yul, _evm15);
	compiler.run(_object);
}

void EVMObjectCompiler::run(Object& _object)
{
	map<YulString, AbstractAssembly::SubID> subIDs;

	for (auto& subNode: _object.subObjects)
		if (Object* subObject = dynamic_cast<Object*>(subNode.get()))
		{
			auto subAssemblyAndID = m_assembly.createSubAssembly();
			subIDs[subObject->name] = subAssemblyAndID.second;
			compile(*subObject, *subAssemblyAndID.first, m_yul, m_evm15);
		}
		else
		{
			Data const& data = dynamic_cast<Data const&>(*subNode);
			subIDs[data.name] = m_assembly.appendData(data.data);
		}

	yulAssert(_object.analysisInfo, "No analysis info.");
	CodeTransform{m_assembly, *_object.analysisInfo, m_yul, m_evm15}(*_object.code);
}
