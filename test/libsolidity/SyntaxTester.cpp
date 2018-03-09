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

#include <test/libsolidity/SyntaxTester.h>
#include <test/libsolidity/AnalysisFramework.h>
#include <test/TestHelper.h>
#include <boost/algorithm/string/replace.hpp>

using namespace dev;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

#if BOOST_VERSION < 105900
test_case *make_test_case(
	function<void()> const& _fn,
	string const& _name,
	string const&, // _filename
	size_t // _line
)
{
	return make_test_case(_fn, _name);
}
#endif


void SyntaxTester::runTest(SyntaxTest const& _test)
{
	vector<string> unexpectedErrors;
	auto expectations = _test.expectations;
	auto errorList = parseAnalyseAndReturnError(_test.source, true, true, true).second;

	bool errorsMatch = true;

	if (errorList.size() != expectations.size())
		errorsMatch = false;
	else
	{
		for (size_t i = 0; i < errorList.size(); i++)
		{
			if (
				!(errorList[i]->typeName() == expectations[i].type) ||
				!(errorMessage(*errorList[i]) == expectations[i].message)
			)
			{
				errorsMatch = false;
				break;
			}
		}
	}

	if (!errorsMatch)
	{
		string msg = "Test expectation mismatch.\nExpected result:\n";
		if (expectations.empty())
			msg += "\tSuccess\n";
		else
			for (auto const& expectation: expectations)
				msg += "\t" + expectation.type + ": " + expectation.message + "\n";
		msg += "Obtained result:\n";
		if (errorList.empty())
			msg += "\tSuccess\n";
		else
			for (auto const& error: errorList)
				msg += "\t" + error->typeName() + ": " + errorMessage(*error) + "\n";
		BOOST_ERROR(msg);
	}
}

std::string SyntaxTester::errorMessage(Error const& _e)
{
	if (_e.comment())
		return boost::replace_all_copy(*_e.comment(), "\n", "\\n");
	else
		return "NONE";
}

int SyntaxTester::registerTests(
	test_suite& _suite,
	fs::path const& _basepath,
	fs::path const& _path
)
{

	int numTestsAdded = 0;
	fs::path fullpath = _basepath / _path;
	if (fs::is_directory(fullpath))
	{
		test_suite* sub_suite = BOOST_TEST_SUITE(_path.filename().string());
		for (auto const& entry: boost::iterator_range<fs::directory_iterator>(
			fs::directory_iterator(fullpath),
			fs::directory_iterator()
		))
			numTestsAdded += registerTests(*sub_suite, _basepath, _path / entry.path().filename());
		_suite.add(sub_suite);
	}
	else
	{
		_suite.add(make_test_case(
			[fullpath] { SyntaxTester().runTest(SyntaxTestParser().parse(fullpath.string())); },
			_path.stem().string(),
			_path.string(),
			0
		));
		numTestsAdded = 1;
	}
	return numTestsAdded;
}

void SyntaxTester::registerTests()
{
	if(dev::test::Options::get().testPath.empty())
		throw runtime_error(
			"No path to the test files was specified. "
			"Use the --testpath command line option or "
			"the ETH_TEST_PATH environment variable."
		);
	auto testPath = fs::path(dev::test::Options::get().testPath);

	if (fs::exists(testPath) && fs::is_directory(testPath))
	{
		int numTestsAdded = registerTests(
			framework::master_test_suite(),
			testPath / "libsolidity",
			"syntaxTests"
		);
		solAssert(numTestsAdded > 0, "no syntax tests found in libsolidity/syntaxTests");
	}
	else
		solAssert(false, "libsolidity/syntaxTests directory not found");
}
