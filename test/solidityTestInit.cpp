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
/** @file boostTest.cpp
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 * Stub for generating main boost.test module.
 * Original code taken from boost sources.
 */


#include <test/libtestcore/boostTest.h>
#include <test/libtestcore/Options.h>

#include <test/libsolidity/ASTJSONTest.h>
#include <test/libsolidity/SyntaxTest.h>
#include <test/libsolidity/SMTCheckerJSONTest.h>
#include <test/libyul/YulOptimizerTest.h>

using namespace std;
using namespace boost::unit_test;
using namespace dev::solidity::test;

using namespace dev::test;

/**
 *  Entry-Point for the Boost Test Suite.
 */
test_suite* init_unit_test_suite( int /*argc*/, char* /*argv*/[] )
{
	master_test_suite_t& master = framework::master_test_suite();
	master.p_name.value = "SolidityTests";
	dev::test::Options::get().validate();
	solAssert(registerTests(
		master,
		dev::test::Options::get().testPath / "libsolidity",
		"syntaxTests",
		SyntaxTest::create
	) > 0, "no syntax tests found");
	solAssert(registerTests(
		master,
		dev::test::Options::get().testPath / "libsolidity",
		"ASTJSON",
		ASTJSONTest::create
	) > 0, "no JSON AST tests found");
	solAssert(registerTests(
		master,
		dev::test::Options::get().testPath / "libyul",
		"yulOptimizerTests",
		yul::test::YulOptimizerTest::create
	) > 0, "no Yul Optimizer tests found");
	if (!dev::test::Options::get().disableSMT)
	{
		solAssert(registerTests(
			master,
			dev::test::Options::get().testPath / "libsolidity",
			"smtCheckerTests",
			SyntaxTest::create
		) > 0, "no SMT checker tests found");

		solAssert(registerTests(
			master,
			dev::test::Options::get().testPath / "libsolidity",
			"smtCheckerTestsJSON",
			SMTCheckerTest::create
		) > 0, "no SMT checker JSON tests found");
	}
	if (dev::test::Options::get().disableIPC)
	{
		for (auto suite: {
			"ABIDecoderTest",
			"ABIEncoderTest",
			"SolidityAuctionRegistrar",
			"SolidityFixedFeeRegistrar",
			"SolidityWallet",
#if HAVE_LLL
			"LLLERC20",
			"LLLENS",
			"LLLEndToEndTest",
#endif
			"GasMeterTests",
			"SolidityEndToEndTest",
			"SolidityOptimizer"
		})
			removeTestSuite(suite);
	}
	if (dev::test::Options::get().disableSMT)
		removeTestSuite("SMTChecker");

	return 0;
}
