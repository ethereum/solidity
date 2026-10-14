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
#include <test/TestCase.h>

#include <libsolutil/AnsiColorized.h>
#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string/predicate.hpp>

#include <iostream>
#include <stdexcept>

using namespace solidity;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace solidity::util;

void TestCase::printSettings(std::ostream& _stream, const std::string& _linePrefix, const bool)
{
	auto& settings = m_reader.settings();
	if (settings.empty())
		return;

	_stream << _linePrefix << "// ====" << std::endl;
	for (auto const& setting: settings)
		_stream << _linePrefix << "// " << setting.first << ": " << setting.second << std::endl;
}

void TestCase::printUpdatedSettings(std::ostream& _stream, std::string const& _linePrefix)
{
	printSettings(_stream, _linePrefix);
}

bool TestCase::isTestFilename(boost::filesystem::path const& _filename)
{
	std::string extension = _filename.extension().string();
	// NOTE: .asmjson rather than .json because JSON files that do not represent test cases exist in some test dirs.
	bool const isAllowedExtension =
		extension == ".sol" ||
		extension == ".yul" ||
		extension == ".asm" ||
		extension == ".asmjson" ||
		extension == ".stack" ||
		extension == ".ssa";
	return isAllowedExtension && !_filename.string().starts_with('~') && !_filename.string().starts_with('.');
}

bool TestCase::shouldRun()
{
	m_reader.ensureAllSettingsRead();
	return m_shouldRun;
}

void TestCase::expect(std::string::iterator& _it, std::string::iterator _end, std::string::value_type _c)
{
	if (_it == _end || *_it != _c)
		BOOST_THROW_EXCEPTION(std::runtime_error(std::string("Invalid test expectation. Expected: \"") + _c + "\"."));
	++_it;
}

void TestCase::printSource(std::ostream& _stream, std::string const& _linePrefix, bool const) const
{
	printPrefixed(_stream, m_source, _linePrefix);
}

void TestCase::printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const
{
	printPrefixed(_stream, m_obtainedResult, _linePrefix);
}

TestCase::TestResult TestCase::checkResult(std::ostream& _stream, const std::string& _linePrefix, bool const _formatted)
{
	if (m_expectation != m_obtainedResult)
	{
		std::string nextIndentLevel = _linePrefix + "  ";
		util::AnsiColorized(_stream, _formatted, {util::formatting::BOLD, util::formatting::CYAN})
			<< _linePrefix << "Expected result:" << std::endl;
		// TODO could compute a simple diff with highlighted lines
		printPrefixed(_stream, m_expectation, nextIndentLevel);
		util::AnsiColorized(_stream, _formatted, {util::formatting::BOLD, util::formatting::CYAN})
			<< _linePrefix << "Obtained result:" << std::endl;
		printPrefixed(_stream, m_obtainedResult, nextIndentLevel);
		return TestResult::Failure;
	}
	return TestResult::Success;
}

void EVMVersionRestrictedTestCase::processEVMVersionSetting()
{
	std::string versionString = m_reader.stringSetting("EVMVersion", "any");
	if (versionString == "any")
		return;

	std::string comparator;
	size_t versionBegin = 0;
	for (auto character: versionString)
		if (!isalpha(character, std::locale::classic()) && character != '@')
		{
			comparator += character;
			versionBegin++;
		}
		else
			break;

	versionString = versionString.substr(versionBegin);
	std::optional<langutil::EVMVersion> version;
	if (versionString == "current")
		version = std::make_optional<langutil::EVMVersion>();
	else
		version = langutil::EVMVersion::fromString(versionString);
	if (!version)
		BOOST_THROW_EXCEPTION(std::runtime_error{"Invalid EVM version: \"" + versionString + "\""});

	langutil::EVMVersion evmVersion = solidity::test::CommonOptions::get().evmVersion();
	bool comparisonResult;
	if (comparator == ">")
		comparisonResult = evmVersion > version;
	else if (comparator == ">=")
		comparisonResult = evmVersion >= version;
	else if (comparator == "<")
		comparisonResult = evmVersion < version;
	else if (comparator == "<=")
		comparisonResult = evmVersion <= version;
	else if (comparator == "=")
		comparisonResult = evmVersion == version;
	else if (comparator == "!")
		comparisonResult = !(evmVersion == version);
	else
		BOOST_THROW_EXCEPTION(std::runtime_error{"Invalid EVM comparator: \"" + comparator + "\""});

	if (!comparisonResult)
		m_shouldRun = false;
}

EVMVersionRestrictedTestCase::EVMVersionRestrictedTestCase(std::string const& _filename):
	TestCase(_filename)
{
	processEVMVersionSetting();
}
