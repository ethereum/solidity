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

#pragma once

#include <test/TestCaseReader.h>

#include <liblangutil/EVMVersion.h>

#include <boost/filesystem.hpp>

#include <string>

namespace solidity::frontend::test
{

/**
 * Common superclass of anything that can be run via isoltest.
 */
class TestCase
{
public:
	struct Config
	{
		std::string filename;
		langutil::EVMVersion evmVersion;
		std::optional<uint8_t> eofVersion;
		std::vector<boost::filesystem::path> vmPaths;
		bool enforceGasCost = false;
		u256 enforceGasCostMinValue;
	};

	enum class TestResult { Success, Failure, FatalError };

	using TestCaseCreator = std::unique_ptr<TestCase>(*)(Config const&);

	virtual ~TestCase() = default;

	/// Runs the test case.
	/// Outputs error messages to @arg _stream. Each line of output is prefixed with @arg _linePrefix.
	/// Optionally, color-coding can be enabled (if @arg _formatted is set to true).
	/// @returns true, if the test case succeeds, false otherwise
	virtual TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool const _formatted = false) = 0;

	/// Outputs the test contract to @arg _stream.
	/// Each line of output is prefixed with @arg _linePrefix.
	/// If @arg _formatted is true, color-coding may be used to indicate
	/// error locations in the contract, if applicable.
	virtual void printSource(std::ostream &_stream, std::string const &_linePrefix = "", bool const _formatted = false) const;
	/// Outputs settings.
	virtual void printSettings(std::ostream &_stream, std::string const &_linePrefix = "", bool const _formatted = false);
	/// Outputs updated settings
	virtual void printUpdatedSettings(std::ostream& _stream, std::string const& _linePrefix = "");
	/// Outputs test expectations to @arg _stream that match the actual results of the test.
	/// Each line of output is prefixed with @arg _linePrefix.
	virtual void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const;

	static bool isTestFilename(boost::filesystem::path const& _filename);

	/// Returns true, if the test case is supported in the current environment and false
	/// otherwise which causes this test to be skipped.
	/// This might check e.g. for restrictions on the EVM version.
	/// The function throws an exception if there are unread settings.
	bool shouldRun();

protected:
	// Used by ASTJSONTest, the only TestCase class with a custom parser of the test files.
	TestCase() = default;

	TestCase(std::string const& _filename): m_reader(_filename) {}

	static void expect(std::string::iterator& _it, std::string::iterator _end, std::string::value_type _c);

	template<typename IteratorType>
	static void skipWhitespace(IteratorType& _it, IteratorType _end)
	{
		while (_it != _end && std::isspace<char>(*_it, std::locale::classic()))
			++_it;
	}

	template<typename IteratorType>
	static void skipSlashes(IteratorType& _it, IteratorType _end)
	{
		while (_it != _end && *_it == '/')
			++_it;
	}

	TestCase::TestResult checkResult(std::ostream& _stream, const std::string& _linePrefix, bool const _formatted);

	std::string m_source;
	std::string m_obtainedResult;
	std::string m_expectation;

	TestCaseReader m_reader;
	bool m_shouldRun = true;
};

class EVMVersionRestrictedTestCase: public TestCase
{
protected:
	EVMVersionRestrictedTestCase(std::string const& _filename);
};

}
