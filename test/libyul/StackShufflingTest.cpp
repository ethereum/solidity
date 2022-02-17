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
#include <libyul/backends/evm/StackHelpers.h>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity::langutil;

namespace solidity::yul::test
{

BOOST_AUTO_TEST_SUITE(YulStackShuffling)

BOOST_AUTO_TEST_CASE(swap_cycle)
{
	std::vector<Scope::Variable> scopeVariables;
	Scope::Function function;
	std::vector<VariableSlot> v;
	for (size_t i = 0; i < 17; ++i)
		scopeVariables.emplace_back(Scope::Variable{""_yulstring, YulString{"v" + to_string(i)}});
	for (size_t i = 0; i < 17; ++i)
		v.emplace_back(VariableSlot{scopeVariables[i]});

	Stack sourceStack{
		v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[9], v[10], v[11], v[12], v[13], v[14], v[15], v[16],
		FunctionReturnLabelSlot{function}, FunctionReturnLabelSlot{function}, v[5]};
	Stack targetStack{
		v[1], v[0], v[2], v[3], v[4], v[5], v[6], v[7], v[9], v[10], v[11], v[12], v[13], v[14], v[15], v[16],
		FunctionReturnLabelSlot{function}, JunkSlot{}, JunkSlot{}
	};
	// Used to hit a swapping cycle.
	createStackLayout(sourceStack, targetStack, [](auto){}, [](auto){}, [](){});
}

BOOST_AUTO_TEST_SUITE_END()

}
