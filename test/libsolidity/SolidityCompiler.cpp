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
 * Unit tests for the compiler itself.
 */

#include <test/libsolidity/AnalysisFramework.h>
#include <test/Metadata.h>
#include <test/Common.h>

#include <boost/test/unit_test.hpp>


namespace solidity::frontend::test
{

class SolidityCompilerFixture: protected AnalysisFramework
{
	void setupCompiler(CompilerStack& _compiler) override
	{
		// FIXME: This test was probably supposed to respect CommonOptions::get().optimize but
		// due to a bug it was always running with optimizer disabled and it does not pass with it.
		_compiler.setOptimiserSettings(false);
	}
};

BOOST_FIXTURE_TEST_SUITE(SolidityCompiler, SolidityCompilerFixture)

BOOST_AUTO_TEST_CASE(does_not_include_creation_time_only_internal_functions)
{
	char const* sourceCode = R"(
		contract C {
			uint x;
			constructor() { f(); }
			function f() internal { unchecked { for (uint i = 0; i < 10; ++i) x += 3 + i; } }
		}
	)";

	runFramework(sourceCode, PipelineStage::Compilation);
	BOOST_REQUIRE_MESSAGE(
		pipelineSuccessful(),
		"Contract compilation failed:\n" + formatErrors(filteredErrors(), true /* _colored */)
	);

	bytes const& creationBytecode = solidity::test::bytecodeSansMetadata(compiler().object("C").bytecode);
	bytes const& runtimeBytecode = solidity::test::bytecodeSansMetadata(compiler().runtimeObject("C").bytecode);
	BOOST_CHECK(creationBytecode.size() >= 90);
	BOOST_CHECK(creationBytecode.size() <= 120);
	auto evmVersion = solidity::test::CommonOptions::get().evmVersion();
	unsigned threshold = evmVersion.hasPush0() ? 9 : 10;
	BOOST_CHECK(runtimeBytecode.size() >= threshold);
	BOOST_CHECK(runtimeBytecode.size() <= 30);
}

BOOST_AUTO_TEST_SUITE_END()

}
