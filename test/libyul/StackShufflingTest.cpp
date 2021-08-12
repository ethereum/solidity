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
 * Unit tests for stack shuffling.
 */
#include <test/Common.h>

#include <libyul/backends/evm/StackHelpers.h>
#include <libyul/backends/evm/OptimizedEVMCodeTransform.h>

#include <boost/test/unit_test.hpp>

using namespace std;

namespace solidity::yul::test
{

namespace
{
bool triggersStackTooDeep(Stack _source, Stack const& _target)
{
	bool result = false;
	createStackLayout(_source, _target, [&](unsigned _i) {
		if (_i > 16)
			result = true;
	}, [&](StackSlot const& _slot) {
		if (canBeFreelyGenerated(_slot))
			return;
		if (auto depth = util::findOffset(_source | ranges::views::reverse, _slot); depth && *depth >= 16)
			if (*depth >= 16)
				result = true;
	}, [&]() {});
	return result;
}
}

BOOST_AUTO_TEST_SUITE(StackHelpers)

BOOST_AUTO_TEST_CASE(avoid_deep_dup)
{
	vector<Scope::Variable> variableContainer;
	for (size_t i = 0; i < 15; ++i)
		variableContainer.emplace_back(Scope::Variable{YulString{}, YulString("v" + to_string(i))});
	Stack from = {{
		VariableSlot{variableContainer[0]},
		VariableSlot{variableContainer[1]},
		VariableSlot{variableContainer[2]},
		VariableSlot{variableContainer[3]},
		VariableSlot{variableContainer[4]},
		VariableSlot{variableContainer[5]},
		VariableSlot{variableContainer[6]},
		VariableSlot{variableContainer[7]},
		VariableSlot{variableContainer[8]},
		VariableSlot{variableContainer[9]},
		VariableSlot{variableContainer[10]},
		VariableSlot{variableContainer[11]},
		VariableSlot{variableContainer[12]},
		VariableSlot{variableContainer[12]},
		VariableSlot{variableContainer[13]},
		VariableSlot{variableContainer[14]}
	}};
	Stack to = {{
		VariableSlot{variableContainer[0]},
		VariableSlot{variableContainer[1]},
		VariableSlot{variableContainer[2]},
		VariableSlot{variableContainer[3]},
		VariableSlot{variableContainer[4]},
		VariableSlot{variableContainer[5]},
		VariableSlot{variableContainer[6]},
		VariableSlot{variableContainer[7]},
		VariableSlot{variableContainer[8]},
		VariableSlot{variableContainer[9]},
		VariableSlot{variableContainer[10]},
		VariableSlot{variableContainer[11]},
		VariableSlot{variableContainer[12]},
		VariableSlot{variableContainer[12]},
		VariableSlot{variableContainer[13]},
		VariableSlot{variableContainer[14]},
		VariableSlot{variableContainer[14]}, // While "optimal", bringing this slot up first will make the next unreachable.
		VariableSlot{variableContainer[0]}
	}};
	BOOST_CHECK(!triggersStackTooDeep(from, to));
}

BOOST_AUTO_TEST_SUITE_END()

} // end namespaces
