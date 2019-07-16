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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4535) // calling _set_se_translator requires /EHa
#endif
#include <boost/test/unit_test.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#pragma GCC diagnostic pop

#include <test/InteractiveTests.h>
#include <test/Options.h>
#include <test/EVMHost.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <string>

using namespace boost::unit_test;
using namespace dev::solidity::test;
namespace fs = boost::filesystem;
using namespace std;

#if BOOST_VERSION < 105900
test_case *make_test_case(
	function<void()> const& _fn,
	string const& _name,
	string const& /* _filename */,
	size_t /* _line */
)
{
	return make_test_case(_fn, _name);
}
#endif

namespace
{
void removeTestSuite(std::string const& _name)
{
	master_test_suite_t& master = framework::master_test_suite();
	auto id = master.get(_name);
	assert(id != INV_TEST_UNIT_ID);
	master.remove(id);
}

int registerTests(
	boost::unit_test::test_suite& _suite,
	boost::filesystem::path const& _basepath,
	boost::filesystem::path const& _path,
	std::string const& _ipcPath,
	TestCase::TestCaseCreator _testCaseCreator
)
{
	int numTestsAdded = 0;
	fs::path fullpath = _basepath / _path;
	TestCase::Config config{fullpath.string(), _ipcPath, dev::test::Options::get().evmVersion()};
	if (fs::is_directory(fullpath))
	{
		test_suite* sub_suite = BOOST_TEST_SUITE(_path.filename().string());
		for (auto const& entry: boost::iterator_range<fs::directory_iterator>(
			fs::directory_iterator(fullpath),
			fs::directory_iterator()
		))
			if (fs::is_directory(entry.path()) || TestCase::isTestFilename(entry.path().filename()))
				numTestsAdded += registerTests(*sub_suite, _basepath, _path / entry.path().filename(), _ipcPath, _testCaseCreator);
		_suite.add(sub_suite);
	}
	else
	{
		static vector<unique_ptr<string>> filenames;

		filenames.emplace_back(new string(_path.string()));
		_suite.add(make_test_case(
			[config, _testCaseCreator]
			{
				BOOST_REQUIRE_NO_THROW({
					try
					{
						stringstream errorStream;
						auto testCase = _testCaseCreator(config);
						if (testCase->validateSettings(dev::test::Options::get().evmVersion()))
							switch (testCase->run(errorStream))
							{
								case TestCase::TestResult::Success:
									break;
								case TestCase::TestResult::Failure:
									BOOST_ERROR("Test expectation mismatch.\n" + errorStream.str());
									break;
								case TestCase::TestResult::FatalError:
									BOOST_ERROR("Fatal error during test.\n" + errorStream.str());
									break;
							}
					}
					catch (boost::exception const& _e)
					{
						BOOST_ERROR("Exception during extracted test: " << boost::diagnostic_information(_e));
					}
			   });
			},
			_path.stem().string(),
			*filenames.back(),
			0
		));
		numTestsAdded = 1;
	}
	return numTestsAdded;
}
}

test_suite* init_unit_test_suite( int /*argc*/, char* /*argv*/[] )
{
	master_test_suite_t& master = framework::master_test_suite();
	master.p_name.value = "SolidityTests";
	dev::test::Options::get().validate();

	bool disableIPC = !dev::test::EVMHost::getVM();
	if (disableIPC)
	{
		cout << "Unable to find libevmone.so. Please provide the path using -- --evmonepath <path>." << endl;
		cout << "You can download it at" << endl;
		cout << "https://github.com/ethereum/evmone/releases/download/v0.1.0/evmone-0.1.0-linux-x86_64.tar.gz" << endl;
		cout << endl << "--- SKIPPING ALL SEMANTICS TESTS ---" << endl << endl;
	}
	// Include the interactive tests in the automatic tests as well
	for (auto const& ts: g_interactiveTestsuites)
	{
		auto const& options = dev::test::Options::get();

		if (ts.smt && options.disableSMT)
			continue;

		if (ts.ipc && disableIPC)
			continue;

		solAssert(registerTests(
			master,
			options.testPath / ts.path,
			ts.subpath,
			options.evmonePath.string(),
			ts.testCaseCreator
		) > 0, std::string("no ") + ts.title + " tests found");
	}

	if (disableIPC)
	{
		for (auto suite: {
			"ABIDecoderTest",
			"ABIEncoderTest",
			"SolidityAuctionRegistrar",
			"SolidityFixedFeeRegistrar",
			"SolidityWallet",
#if HAVE_LLL
			"LLLERC20",
			"LLLENS",
			"LLLEndToEndTest",
#endif
			"GasMeterTests",
			"GasCostTests",
			"SolidityEndToEndTest",
			"SolidityOptimizer"
		})
			removeTestSuite(suite);
	}

	if (dev::test::Options::get().disableSMT)
		removeTestSuite("SMTChecker");

	return 0;
}

// BOOST_TEST_DYN_LINK should be defined if user want to link against shared boost test library
#ifdef BOOST_TEST_DYN_LINK

// Because we want to have customized initialization function and support shared boost libraries at the same time,
// we are forced to customize the entry point.
// see: https://www.boost.org/doc/libs/1_67_0/libs/test/doc/html/boost_test/adv_scenarios/shared_lib_customizations/init_func.html

int main(int argc, char* argv[])
{
	auto init_unit_test = []() -> bool { init_unit_test_suite(0, nullptr); return true; };
	return boost::unit_test::unit_test_main(init_unit_test, argc, argv);
}

#endif
