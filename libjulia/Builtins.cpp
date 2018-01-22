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
 * Utility module that provides the list of builtin functions.
 */

#include <libjulia/Builtins.h>

#include <libevmasm/Instruction.h>

using namespace std;
using namespace dev;
using namespace dev::julia;


BuiltinFunctions::BuiltinFunctions(bool _iulia)
{
	if (_iulia)
		return; // TODO
	for (auto const& instruction: solidity::c_instructions)
	{
		if (
			instruction.second == solidity::Instruction::JUMPDEST ||
			solidity::isPushInstruction(instruction.second) ||
			solidity::isSwapInstruction(instruction.second) ||
			solidity::isDupInstruction(instruction.second)
		)
			continue;
		string name = instruction.first;
		transform(name.begin(), name.end(), name.begin(), [](unsigned char _c) { return tolower(_c); });
		m_builtinFunctions.insert(std::move(name));
	}
}

set<string> const& BuiltinFunctions::names(bool _iulia)
{
	return instance(_iulia).m_builtinFunctions;
}

BuiltinFunctions const& BuiltinFunctions::instance(bool _iulia)
{
	static BuiltinFunctions assembly(true);
	static BuiltinFunctions iulia(true);
	return _iulia ? iulia : assembly;
}
