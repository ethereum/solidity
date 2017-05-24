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
 * @date 2017
 * Unit tests for the metadata output.
 */

#include "../Metadata.h"
#include "../TestHelper.h"
#include <libsolidity/interface/CompilerStack.h>
#include <libdevcore/SwarmHash.h>

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_AUTO_TEST_SUITE(Metadata)

BOOST_AUTO_TEST_CASE(metadata_stamp)
{
	// Check that the metadata stamp is at the end of the runtime bytecode.
	char const* sourceCode = R"(
		pragma solidity >=0.0;
		contract test {
			function g(function(uint) external returns (uint) x) {}
		}
	)";
	CompilerStack compilerStack;
	BOOST_REQUIRE(compilerStack.compile(std::string(sourceCode)));
	bytes const& bytecode = compilerStack.runtimeObject("test").bytecode;
	std::string const& metadata = compilerStack.onChainMetadata("test");
	BOOST_CHECK(dev::test::isValidMetadata(metadata));
	bytes hash = dev::swarmHash(metadata).asBytes();
	BOOST_REQUIRE(hash.size() == 32);
	BOOST_REQUIRE(bytecode.size() >= 2);
	size_t metadataCBORSize = (size_t(bytecode.end()[-2]) << 8) + size_t(bytecode.end()[-1]);
	BOOST_REQUIRE(metadataCBORSize < bytecode.size() - 2);
	bytes expectation = bytes{0xa1, 0x65, 'b', 'z', 'z', 'r', '0', 0x58, 0x20} + hash;
	BOOST_CHECK(std::equal(expectation.begin(), expectation.end(), bytecode.end() - metadataCBORSize - 2));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
