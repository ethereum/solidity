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

#include <string>
#include <boost/test/unit_test.hpp>
#include <libsolidity/interface/Exceptions.h>
#include <libsolidity/interface/CompilerStack.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

BOOST_AUTO_TEST_SUITE(SolidityImports)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CompilerStack c;
	c.addSource("a", "contract C {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(regular_import)
{
	CompilerStack c;
	c.addSource("a", "contract C {} pragma solidity >=0.0;");
	c.addSource("b", "import \"a\"; contract D is C {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(import_does_not_clutter_importee)
{
	CompilerStack c;
	c.addSource("a", "contract C { D d; } pragma solidity >=0.0;");
	c.addSource("b", "import \"a\"; contract D is C {} pragma solidity >=0.0;");
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(import_is_transitive)
{
	CompilerStack c;
	c.addSource("a", "contract C { } pragma solidity >=0.0;");
	c.addSource("b", "import \"a\"; pragma solidity >=0.0;");
	c.addSource("c", "import \"b\"; contract D is C {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(circular_import)
{
	CompilerStack c;
	c.addSource("a", "import \"b\"; contract C { D d; } pragma solidity >=0.0;");
	c.addSource("b", "import \"a\"; contract D { C c; } pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(relative_import)
{
	CompilerStack c;
	c.addSource("a", "import \"./dir/b\"; contract A is B {} pragma solidity >=0.0;");
	c.addSource("dir/b", "contract B {} pragma solidity >=0.0;");
	c.addSource("dir/c", "import \"../a\"; contract C is A {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(relative_import_multiplex)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma solidity >=0.0;");
	c.addSource("dir/a/b/c", "import \"../../.././a\"; contract B is A {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(simple_alias)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma solidity >=0.0;");
	c.addSource("dir/a/b/c", "import \"../../.././a\" as x; contract B is x.A { function() { x.A r = x.A(20); } } pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(library_name_clash)
{
	CompilerStack c;
	c.addSource("a", "library A {} pragma solidity >=0.0;");
	c.addSource("b", "library A {} pragma solidity >=0.0;");
	c.addSource("c", "import {A} from \"./a\"; import {A} from \"./b\";");
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(library_name_clash_with_contract)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma solidity >=0.0;");
	c.addSource("b", "library A {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(complex_import)
{
	CompilerStack c;
	c.addSource("a", "contract A {} contract B {} contract C { struct S { uint a; } } pragma solidity >=0.0;");
	c.addSource("b", "import \"a\" as x; import {B as b, C as c, C} from \"a\"; "
				"contract D is b { function f(c.S var1, x.C.S var2, C.S var3) internal {} } pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(name_clash_in_import)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma solidity >=0.0;");
	c.addSource("b", "import \"a\"; contract A {} pragma solidity >=0.0;");
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import \"a\" as A; contract A {} pragma solidity >=0.0;");
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import {A as b} from \"a\"; contract b {} pragma solidity >=0.0;");
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import {A} from \"a\"; contract A {} pragma solidity >=0.0;");
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import {A} from \"a\"; contract B {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(remappings)
{
	CompilerStack c;
	c.setRemappings(vector<string>{"s=s_1.4.6", "t=Tee"});
	c.addSource("a", "import \"s/s.sol\"; contract A is S {} pragma solidity >=0.0;");
	c.addSource("b", "import \"t/tee.sol\"; contract A is Tee {} pragma solidity >=0.0;");
	c.addSource("s_1.4.6/s.sol", "contract S {} pragma solidity >=0.0;");
	c.addSource("Tee/tee.sol", "contract Tee {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings)
{
	CompilerStack c;
	c.setRemappings(vector<string>{"a:s=s_1.4.6", "b:s=s_1.4.7"});
	c.addSource("a/a.sol", "import \"s/s.sol\"; contract A is SSix {} pragma solidity >=0.0;");
	c.addSource("b/b.sol", "import \"s/s.sol\"; contract B is SSeven {} pragma solidity >=0.0;");
	c.addSource("s_1.4.6/s.sol", "contract SSix {} pragma solidity >=0.0;");
	c.addSource("s_1.4.7/s.sol", "contract SSeven {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(filename_with_period)
{
	CompilerStack c;
	c.addSource("a/a.sol", "import \".b.sol\"; contract A is B {} pragma solidity >=0.0;");
	c.addSource("a/.b.sol", "contract B {} pragma solidity >=0.0;");
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_ensure_default_and_module_preserved)
{
	CompilerStack c;
	c.setRemappings(vector<string>{"foo=vendor/foo_2.0.0", "vendor/bar:foo=vendor/foo_1.0.0", "bar=vendor/bar"});
	c.addSource("main.sol", "import \"foo/foo.sol\"; import {Bar} from \"bar/bar.sol\"; contract Main is Foo2, Bar {} pragma solidity >=0.0;");
	c.addSource("vendor/bar/bar.sol", "import \"foo/foo.sol\"; contract Bar {Foo1 foo;} pragma solidity >=0.0;");
	c.addSource("vendor/foo_1.0.0/foo.sol", "contract Foo1 {} pragma solidity >=0.0;");
	c.addSource("vendor/foo_2.0.0/foo.sol", "contract Foo2 {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_order_independent)
{
	CompilerStack c;
	c.setRemappings(vector<string>{"a:x/y/z=d", "a/b:x=e"});
	c.addSource("a/main.sol", "import \"x/y/z/z.sol\"; contract Main is D {} pragma solidity >=0.0;");
	c.addSource("a/b/main.sol", "import \"x/y/z/z.sol\"; contract Main is E {} pragma solidity >=0.0;");
	c.addSource("d/z.sol", "contract D {} pragma solidity >=0.0;");
	c.addSource("e/y/z/z.sol", "contract E {} pragma solidity >=0.0;");
	BOOST_CHECK(c.compile());
	CompilerStack d;
	d.setRemappings(vector<string>{"a/b:x=e", "a:x/y/z=d"});
	d.addSource("a/main.sol", "import \"x/y/z/z.sol\"; contract Main is D {} pragma solidity >=0.0;");
	d.addSource("a/b/main.sol", "import \"x/y/z/z.sol\"; contract Main is E {} pragma solidity >=0.0;");
	d.addSource("d/z.sol", "contract D {} pragma solidity >=0.0;");
	d.addSource("e/y/z/z.sol", "contract E {} pragma solidity >=0.0;");
	BOOST_CHECK(d.compile());
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
