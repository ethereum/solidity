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

#pragma once

#include <test/libsolidity/AnalysisFramework.h>
#include <test/libsolidity/SyntaxTestParser.h>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

namespace dev
{
namespace solidity
{
namespace test
{

class SyntaxTester: public AnalysisFramework
{
public:
	static void registerTests();
private:
	static int registerTests(
		boost::unit_test::test_suite& _suite,
		boost::filesystem::path const& _basepath,
		boost::filesystem::path const& _path
	);
	static std::string errorMessage(Error const& _e);
	void runTest(SyntaxTest const& _test);
};

}
}
}
