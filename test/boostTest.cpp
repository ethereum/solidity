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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4535) // calling _set_se_translator requires /EHa
#endif
#include <boost/test/included/unit_test.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#pragma GCC diagnostic pop

#include <test/TestHelper.h>
#include <test/libsolidity/SyntaxTest.h>

using namespace boost::unit_test;

namespace
{
void removeTestSuite(std::string const& _name)
{
	master_test_suite_t& master = framework::master_test_suite();
	auto id = master.get(_name);
	assert(id != INV_TEST_UNIT_ID);
	master.remove(id);
}
}

test_suite* init_unit_test_suite( int /*argc*/, char* /*argv*/[] )
{
	master_test_suite_t& master = framework::master_test_suite();
	master.p_name.value = "SolidityTests";
	solAssert(dev::solidity::test::SyntaxTest::registerTests(
		master,
		dev::test::Options::get().testPath / "libsolidity",
		"syntaxTests"
	) > 0, "no syntax tests found");
	if (dev::test::Options::get().disableIPC)
	{
		for (auto suite: {
			"ABIDecoderTest",
			"ABIEncoderTest",
			"SolidityAuctionRegistrar",
			"SolidityFixedFeeRegistrar",
			"SolidityWallet",
			"LLLERC20",
			"LLLENS",
			"LLLEndToEndTest",
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
