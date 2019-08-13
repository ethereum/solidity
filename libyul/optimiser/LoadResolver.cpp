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
 * Optimisation stage that replaces expressions of type ``sload(x)`` by the value
 * currently stored in storage, if known.
 */

#include <libyul/optimiser/LoadResolver.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/Semantics.h>
#include <libyul/AsmData.h>

using namespace std;
using namespace dev;
using namespace yul;

void LoadResolver::run(Dialect const& _dialect, Block& _ast)
{
	bool containsMSize = MSizeFinder::containsMSize(_dialect, _ast);
	LoadResolver{_dialect, !containsMSize}(_ast);
}

void LoadResolver::visit(Expression& _e)
{
	if (_e.type() == typeid(FunctionCall))
	{
		FunctionCall const& funCall = boost::get<FunctionCall>(_e);
		if (auto const* builtin = dynamic_cast<EVMDialect const&>(m_dialect).builtin(funCall.functionName.name))
			if (!builtin->parameters.empty() && funCall.arguments.at(0).type() == typeid(Identifier))
			{
				YulString key = boost::get<Identifier>(funCall.arguments.at(0)).name;
				if (
					builtin->instruction == dev::eth::Instruction::SLOAD &&
					m_storage.values.count(key)
				)
				{
					_e = Identifier{locationOf(_e), m_storage.values[key]};
					return;
				}
				else if (
					m_optimizeMLoad &&
					builtin->instruction == dev::eth::Instruction::MLOAD &&
					m_memory.values.count(key)
				)
				{
					_e = Identifier{locationOf(_e), m_memory.values[key]};
					return;
				}
			}
	}
}
