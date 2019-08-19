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
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Tests for high level features like import.
 */

#include <test/libsolidity/ErrorCheck.h>
#include <test/Options.h>

#include <liblangutil/Exceptions.h>
#include <libsolidity/interface/CompilerStack.h>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SolidityImports)

BOOST_AUTO_TEST_CASE(remappings)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{{"", "s", "s_1.4.6"},{"", "t", "Tee"}});
	c.setSources({
		{"a", "import \"s/s.sol\"; contract A is S {} pragma solidity >=0.0;"},
		{"b", "import \"t/tee.sol\"; contract A is Tee {} pragma solidity >=0.0;"},
		{"s_1.4.6/s.sol", "contract S {} pragma solidity >=0.0;"},
		{"Tee/tee.sol", "contract Tee {} pragma solidity >=0.0;"}
	});
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{{"a", "s", "s_1.4.6"}, {"b", "s", "s_1.4.7"}});
	c.setSources({
		{"a/a.sol", "import \"s/s.sol\"; contract A is SSix {} pragma solidity >=0.0;"},
		{"b/b.sol", "import \"s/s.sol\"; contract B is SSeven {} pragma solidity >=0.0;"},
		{"s_1.4.6/s.sol", "contract SSix {} pragma solidity >=0.0;"},
		{"s_1.4.7/s.sol", "contract SSeven {} pragma solidity >=0.0;"}
	});
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_ensure_default_and_module_preserved)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{
		{"", "foo", "vendor/foo_2.0.0"},
		{"vendor/bar", "foo", "vendor/foo_1.0.0"},
		{"", "bar", "vendor/bar"}
	});
	c.setSources({
		{"main.sol", "import \"foo/foo.sol\"; import {Bar} from \"bar/bar.sol\"; contract Main is Foo2, Bar {} pragma solidity >=0.0;"},
		{"vendor/bar/bar.sol", "import \"foo/foo.sol\"; contract Bar {Foo1 foo;} pragma solidity >=0.0;"},
		{"vendor/foo_1.0.0/foo.sol", "contract Foo1 {} pragma solidity >=0.0;"},
		{"vendor/foo_2.0.0/foo.sol", "contract Foo2 {} pragma solidity >=0.0;"}
	});
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_order_independent_1)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{{"a", "x/y/z", "d"}, {"a/b", "x", "e"}});
	c.setSources({
		{"a/main.sol", "import \"x/y/z/z.sol\"; contract Main is D {} pragma solidity >=0.0;"},
		{"a/b/main.sol", "import \"x/y/z/z.sol\"; contract Main is E {} pragma solidity >=0.0;"},
		{"d/z.sol", "contract D {} pragma solidity >=0.0;"},
		{"e/y/z/z.sol", "contract E {} pragma solidity >=0.0;"}
	});
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_order_independent_2)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{{"a/b", "x", "e"}, {"a", "x/y/z", "d"}});
	c.setSources({
		{"a/main.sol", "import \"x/y/z/z.sol\"; contract Main is D {} pragma solidity >=0.0;"},
		{"a/b/main.sol", "import \"x/y/z/z.sol\"; contract Main is E {} pragma solidity >=0.0;"},
		{"d/z.sol", "contract D {} pragma solidity >=0.0;"},
		{"e/y/z/z.sol", "contract E {} pragma solidity >=0.0;"}
	});
	c.setEVMVersion(dev::test::Options::get().evmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
