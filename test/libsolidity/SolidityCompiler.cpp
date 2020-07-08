// SPDX-License-Identifier: GPL-3.0
/**
 * Unit tests for the compiler itself.
 */

#include <test/libsolidity/AnalysisFramework.h>
#include <test/Metadata.h>
#include <test/Common.h>

#include <boost/test/unit_test.hpp>

using namespace std;

namespace solidity::frontend::test
{

BOOST_FIXTURE_TEST_SUITE(SolidityCompiler, AnalysisFramework)

BOOST_AUTO_TEST_CASE(does_not_include_creation_time_only_internal_functions)
{
	char const* sourceCode = R"(
		contract C {
			uint x;
			constructor() public { f(); }
			function f() internal { for (uint i = 0; i < 10; ++i) x += 3 + i; }
		}
	)";
	compiler().setOptimiserSettings(solidity::test::CommonOptions::get().optimize);
	BOOST_REQUIRE(success(sourceCode));
	BOOST_REQUIRE_MESSAGE(compiler().compile(), "Compiling contract failed");
	bytes const& creationBytecode = solidity::test::bytecodeSansMetadata(compiler().object("C").bytecode);
	bytes const& runtimeBytecode = solidity::test::bytecodeSansMetadata(compiler().runtimeObject("C").bytecode);
	BOOST_CHECK(creationBytecode.size() >= 90);
	BOOST_CHECK(creationBytecode.size() <= 120);
	BOOST_CHECK(runtimeBytecode.size() >= 10);
	BOOST_CHECK(runtimeBytecode.size() <= 30);
}

BOOST_AUTO_TEST_SUITE_END()

}
