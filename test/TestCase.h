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

#include <liblangutil/EVMVersion.h>

#include <boost/filesystem.hpp>

#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace dev
{
namespace solidity
{
namespace test
{

#define soltestAssert(CONDITION, DESCRIPTION) \
	do \
	{ \
		if (!(CONDITION)) \
			BOOST_THROW_EXCEPTION(runtime_error(DESCRIPTION)); \
	} \
	while (false)

/**
 * Common superclass of anything that can be run via isoltest.
 */
class TestCase
{
public:
	struct Config
	{
		std::string filename;
		std::string ipcPath;
		langutil::EVMVersion evmVersion;
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
	virtual void printSource(std::ostream &_stream, std::string const &_linePrefix = "", bool const _formatted = false) const = 0;
	/// Outputs the updated settings.
	virtual void printUpdatedSettings(std::ostream &_stream, std::string const &_linePrefix = "", bool const _formatted = false);
	/// Outputs test expectations to @arg _stream that match the actual results of the test.
	/// Each line of output is prefixed with @arg _linePrefix.
	virtual void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const = 0;

	static bool isTestFilename(boost::filesystem::path const& _filename);

	/// Validates the settings, i.e. moves them from m_settings to m_validatedSettings.
	/// Throws a runtime exception if any setting is left at this class (i.e. unknown setting).
	/// Returns true, if the test case is supported in the current environment and false
	/// otherwise which causes this test to be skipped.
	/// This might check e.g. for restrictions on the EVM version.
	virtual bool validateSettings(langutil::EVMVersion /*_evmVersion*/);

protected:
	std::string parseSourceAndSettings(std::istream& _file);
	static void expect(std::string::iterator& _it, std::string::iterator _end, std::string::value_type _c);

	static std::string parseSimpleExpectations(std::istream& _file);

	template<typename IteratorType>
	static void skipWhitespace(IteratorType& _it, IteratorType _end)
	{
		while (_it != _end && isspace(*_it))
			++_it;
	}

	template<typename IteratorType>
	static void skipSlashes(IteratorType& _it, IteratorType _end)
	{
		while (_it != _end && *_it == '/')
			++_it;
	}

	/// Parsed settings.
	std::map<std::string, std::string> m_settings;
	/// Updated settings after validation.
	std::map<std::string, std::string> m_validatedSettings;
};

class EVMVersionRestrictedTestCase: public TestCase
{
public:
	/// Returns true, if the test case is supported for EVM version @arg _evmVersion, false otherwise.
	bool validateSettings(langutil::EVMVersion _evmVersion) override;
};
}
}
}
