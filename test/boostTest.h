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
/** @file boostTest.h
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 * Stub for generating main boost.test module.
 * Original code taken from boost sources.
 */

#pragma once

#include <test/TestCase.h>
#include <boost/test/unit_test.hpp>

namespace dev
{
namespace test
{

extern void removeTestSuite(std::string const& _name);

extern int registerTests(
	boost::unit_test::test_suite& _suite,
	boost::filesystem::path const& _basepath,
	boost::filesystem::path const& _path,
	dev::solidity::test::TestCase::TestCaseCreator _testCaseCreator
);

}
}