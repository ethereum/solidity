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

#include <test/Common.h>
#include <test/FilesystemUtils.h>
#include <test/TestCase.h>

#include <libsolutil/TemporaryDirectory.h>

#include <boost/test/unit_test.hpp>

#include <stdexcept>

using namespace solidity::util;

namespace solidity::frontend::test
{

namespace
{

class EVMVersionSettingTestCase: public EVMVersionRestrictedTestCase
{
public:
	explicit EVMVersionSettingTestCase(std::string const& _filename): EVMVersionRestrictedTestCase(_filename) {}

	TestResult run(std::ostream&, std::string const&, bool const) override { return TestResult::Success; }
};

/// Creates a test file with the given ``EVMVersion`` setting and returns whether the test case should run.
/// Throws if the setting is invalid.
bool shouldRunWithEVMVersionSetting(std::string const& _setting)
{
	TemporaryDirectory tempDir("soltest-evm-version-setting-");
	boost::filesystem::path const testFile = tempDir.path() / "test.sol";
	solidity::test::createFileWithContent(
		testFile,
		"contract C {}\n"
		"// ====\n"
		"// EVMVersion: " + _setting + "\n"
		"// ----\n"
	);
	return EVMVersionSettingTestCase(testFile.string()).shouldRun();
}

langutil::EVMVersion selectedEVMVersion()
{
	return solidity::test::CommonOptions::get().evmVersion();
}

}

BOOST_AUTO_TEST_SUITE(EVMVersionSettingTest)

BOOST_AUTO_TEST_CASE(single_constraint)
{
	BOOST_CHECK_EQUAL(shouldRunWithEVMVersionSetting("=osaka"), selectedEVMVersion() == langutil::EVMVersion::osaka());
	BOOST_CHECK_EQUAL(shouldRunWithEVMVersionSetting("!osaka"), selectedEVMVersion() != langutil::EVMVersion::osaka());
	BOOST_CHECK_EQUAL(shouldRunWithEVMVersionSetting(">=osaka"), selectedEVMVersion() >= langutil::EVMVersion::osaka());
	BOOST_CHECK_EQUAL(shouldRunWithEVMVersionSetting("<osaka"), selectedEVMVersion() < langutil::EVMVersion::osaka());
}

BOOST_AUTO_TEST_CASE(range_with_multiple_versions)
{
	bool const inRange =
		selectedEVMVersion() >= langutil::EVMVersion::london() &&
		selectedEVMVersion() <= langutil::EVMVersion::osaka();
	BOOST_CHECK_EQUAL(shouldRunWithEVMVersionSetting(">=london,<=osaka"), inRange);
	BOOST_CHECK_EQUAL(shouldRunWithEVMVersionSetting(">berlin,<amsterdam"), inRange);
	BOOST_CHECK_EQUAL(shouldRunWithEVMVersionSetting("<=osaka,>=london"), inRange);
}

BOOST_AUTO_TEST_CASE(range_with_all_versions)
{
	BOOST_CHECK(shouldRunWithEVMVersionSetting(">=homestead,<=@future"));
}

BOOST_AUTO_TEST_CASE(range_with_single_version)
{
	BOOST_CHECK_EQUAL(
		shouldRunWithEVMVersionSetting(">=osaka,<=osaka"),
		selectedEVMVersion() == langutil::EVMVersion::osaka()
	);
	BOOST_CHECK_EQUAL(
		shouldRunWithEVMVersionSetting(">prague,<amsterdam"),
		selectedEVMVersion() == langutil::EVMVersion::osaka()
	);
}

BOOST_AUTO_TEST_CASE(empty_range)
{
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">osaka,<osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">osaka,<=osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=amsterdam,<=osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting("=osaka,!osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting("=osaka,=amsterdam"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting("<homestead"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(whitespace_around_commas)
{
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=london, <=osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=london ,<=osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=london  ,   <=osaka"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(constraints_not_separated_by_comma)
{
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=london <=osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=london;<=osaka"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(empty_constraints)
{
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=london,,<=osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=london, ,<=osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(",>=london"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=london,"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(","), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(whitespace_between_comparator_and_version)
{
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">= osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting("= osaka"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=london,<= osaka"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(invalid_constraints)
{
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(""), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting(">=unknownfork"), std::runtime_error);
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting("=>osaka"), std::runtime_error);
	// An invalid constraint is reported even if an earlier one is not satisfied.
	BOOST_CHECK_THROW(shouldRunWithEVMVersionSetting("!osaka,>=unknownfork"), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()

}
