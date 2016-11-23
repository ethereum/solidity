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

#define BOOST_TEST_MODULE EthereumTests
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
