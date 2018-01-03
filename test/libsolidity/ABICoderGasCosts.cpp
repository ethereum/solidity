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
 * Tests for gas costs for the ABI coder.
 */

#include <test/libsolidity/SolidityExecutionFramework.h>

#include <test/libsolidity/ABITestsCommon.h>

#include <libevmasm/GasMeter.h>
#include <libevmasm/KnownState.h>
#include <libevmasm/PathGasMeter.h>
#include <libsolidity/ast/AST.h>
#include <libsolidity/interface/GasEstimator.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>

using namespace std;
using namespace dev::eth;
using namespace dev::solidity;
using namespace dev::test;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(ABICoderGasCosts, SolidityExecutionFramework)

BOOST_AUTO_TEST_CASE(ERC20)
{
	string sourceCode = R"(
		contract ERC20Interface {
			function totalSupply() public constant returns (uint) {}
			function balanceOf(address tokenOwner) public constant returns (uint balance) {}
			function allowance(address tokenOwner, address spender) public constant returns (uint remaining) {}
			function transfer(address to, uint tokens) public returns (bool success) {}
			function approve(address spender, uint tokens) public returns (bool success) {}
			function transferFrom(address from, address to, uint tokens) public returns (bool success) {}
			event Transfer(address indexed from, address indexed to, uint tokens) {}
			event Approval(address indexed tokenOwner, address indexed spender, uint tokens) {}
		}
	)";
	BOTH_ENCODERS(
		compileAndRun(sourceCode);
		ABI_CHECK(callContractFunction("totalSupply()"), encodeArgs(0));
	)
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
