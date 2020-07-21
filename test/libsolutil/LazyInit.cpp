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
// SPDX-License-Identifier: GPL-3.0

#include <libsolutil/LazyInit.h>

#include <boost/test/unit_test.hpp>

#include <utility>

namespace solidity::util::test
{

namespace
{

template<typename T>
void assertInitCalled(LazyInit<T> lazyInit, bool target)
{
	bool initCalled = false;

	lazyInit.init([&]{
	  initCalled = true;
	  return T();
	});

	BOOST_REQUIRE_EQUAL(initCalled, target);
}

// Take ownership to ensure that it doesn't "mutate"
template<typename T>
void assertNotEmpty(LazyInit<T> _lazyInit) { assertInitCalled(std::move(_lazyInit), false); }

// Take ownership to ensure that it doesn't "mutate"
template<typename T>
void assertEmpty(LazyInit<T> _lazyInit) { assertInitCalled(std::move(_lazyInit), true); }

template<typename T>
T valueOf(LazyInit<T> _lazyInit)
{
	return _lazyInit.init([&]{
		BOOST_REQUIRE(false); // this should never be called
		return T();
	});
}

}

BOOST_AUTO_TEST_SUITE(LazyInitTests, *boost::unit_test::label("nooptions"))

BOOST_AUTO_TEST_CASE(default_constructed_is_empty)
{
	assertEmpty(LazyInit<int>());
	assertEmpty(LazyInit<int const>());
}

BOOST_AUTO_TEST_CASE(initialized_is_not_empty)
{
	LazyInit<int> lazyInit;
	lazyInit.init([]{ return 12; });

	assertNotEmpty(std::move(lazyInit));
}

BOOST_AUTO_TEST_CASE(init_returns_init_value)
{
	LazyInit<int> lazyInit;

	BOOST_CHECK_EQUAL(lazyInit.init([]{ return 12; }), 12);

	// A second call to init should not change the value
	BOOST_CHECK_EQUAL(lazyInit.init([]{ return 42; }), 12);
}

BOOST_AUTO_TEST_CASE(moved_from_is_empty)
{
	{
		LazyInit<int> lazyInit;
		{ [[maybe_unused]] auto pilfered = std::move(lazyInit); }

		assertEmpty(std::move(lazyInit));
	}

	{
		LazyInit<int> lazyInit;
		lazyInit.init([]{ return 12; });

		{ [[maybe_unused]] auto pilfered = std::move(lazyInit); }

		assertEmpty(std::move(lazyInit));
	}
}

BOOST_AUTO_TEST_CASE(move_constructed_has_same_value_as_original)
{
	LazyInit<int> original;
	original.init([]{ return 12; });

	LazyInit<int> moveConstructed = std::move(original);
	BOOST_CHECK_EQUAL(valueOf(std::move(moveConstructed)), 12);
}

BOOST_AUTO_TEST_SUITE_END()

}
