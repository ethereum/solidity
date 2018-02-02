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

class ABICoderGasCostsFixture: SolidityExecutionFramework
{
protected:
	void analyze(string const& _sourceCode, vector<string> const& _funsToCall)
	{
		string sourceCode = _sourceCode;
		size_t run = 0;
		int bytecodeSize[2];
		map<string, u256[2]> runGas;
		BOTH_ENCODERS(
			compileAndRun(sourceCode);
			bytecodeSize[run] = m_output.size();
			for (auto const& f: _funsToCall)
			{
				callContractFunction(f);
				runGas[f][run] = m_gasUsed;
			}
			run++;
		)
		bytecodeSize[1] -= 14; // the experimental metadata tag
		cout <<
			"Bytecode size difference: " <<
			(bytecodeSize[1] - bytecodeSize[0]) <<
			" - " <<
			to_string((double)(bytecodeSize[1] - bytecodeSize[0]) / (double)(bytecodeSize[0]) * 100) <<
			"%" <<
			endl;
		for (auto const& f: _funsToCall)
		{
			cout <<
				"Gas difference runtime - " << f << ": " <<
				(runGas[f][1] - runGas[f][0]) <<
				" - " <<
				to_string((double)(runGas[f][1] - runGas[f][0]) / (double)(runGas[f][0]) * 100) <<
				"%" <<
				endl;
		}
	}
	void analyze(string const& _sourceCode, string const& _funToCall = "f()")
	{
		analyze(_sourceCode, vector<string>{_funToCall});
	}
};


BOOST_FIXTURE_TEST_SUITE(ABICoderGasCosts, ABICoderGasCostsFixture)

BOOST_AUTO_TEST_CASE(noop)
{
	string sourceCode = R"(
		contract C {
			function f() public {}
		}
	)";
	analyze(sourceCode);
}

BOOST_AUTO_TEST_CASE(simple_return)
{
	string sourceCode = R"(
		contract C {
			function f() public returns (uint) {}
		}
	)";
	analyze(sourceCode);
}

BOOST_AUTO_TEST_CASE(multi_use)
{
	string sourceCode = R"(
		contract C {
			function f() public returns (uint) {}
			function g() public returns (uint[] x) { x = new uint[](10); x[4] = 2; x[9] = 7; }
			function h() public returns (address[], uint[]) {}
			function i() public returns (uint[], uint) {}
		}
	)";
	analyze(sourceCode, {"f()", "g()", "h()", "i()"});
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
