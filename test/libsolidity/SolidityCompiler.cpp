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

#include <test/Options.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(Compiler, AnalysisFramework)

BOOST_AUTO_TEST_CASE(does_not_include_creation_time_only_internal_functions)
{
	char const* sourceCode = R"(
		contract C {
			uint x;
			constructor() { f(); }
			function f() internal { for (uint i = 0; i < 10; ++i) x += 3 + i; }
		}
	)";
	BOOST_REQUIRE(success(sourceCode));
	m_compiler.setOptimiserSettings(dev::test::Options::get().optimize);
	BOOST_REQUIRE_MESSAGE(m_compiler.compile(), "Compiling contract failed");
	bytes const& creationBytecode = m_compiler.object("C").bytecode;
	bytes const& runtimeBytecode = m_compiler.runtimeObject("C").bytecode;
	BOOST_CHECK(creationBytecode.size() >= 130);
	BOOST_CHECK(creationBytecode.size() <= 160);
	BOOST_CHECK(runtimeBytecode.size() >= 50);
	BOOST_CHECK(runtimeBytecode.size() <= 70);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
