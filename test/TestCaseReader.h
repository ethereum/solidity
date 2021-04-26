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

#include <test/libsolidity/util/SoltestErrors.h>

#include <libsolutil/StringUtils.h>

#include <range/v3/view/map.hpp>

#include <boost/filesystem.hpp>
#include <boost/throw_exception.hpp>

#include <fstream>
#include <map>
#include <string>

namespace solidity::frontend::test
{

/**
 * A map for registering source names that also contains the main source name in a test case.
 */
struct SourceMap
{
	std::map<std::string, std::string> sources;
	std::map<std::string, boost::filesystem::path> externalSources;
	std::string mainSourceFile;
};

/**
 * A reader for test case data file, which parses source, settings and (optionally) simple expectations.
 */
class TestCaseReader
{
public:
	TestCaseReader() = default;
	explicit TestCaseReader(std::string const& _filename);
	explicit TestCaseReader(std::istringstream const& _testCode);

	SourceMap const& sources() const { return m_sources; }
	std::string const& source() const;
	std::size_t lineNumber() const { return m_lineNumber; }
	std::map<std::string, std::string> const& settings() const { return m_settings; }
	std::ifstream& stream() { return m_fileStream; }

	std::string simpleExpectations();

	bool boolSetting(std::string const& _name, bool _defaultValue);
	size_t sizetSetting(std::string const& _name, size_t _defaultValue);
	std::string stringSetting(std::string const& _name, std::string const& _defaultValue);

	template <typename E>
	E enumSetting(std::string const& _name, std::map<std::string, E> const& _choices, std::string const& _defaultChoice);

	void ensureAllSettingsRead() const;

private:
	std::pair<SourceMap, std::size_t> parseSourcesAndSettingsWithLineNumber(std::istream& _file);
	static std::string parseSimpleExpectations(std::istream& _file);

	std::ifstream m_fileStream;
	boost::filesystem::path m_fileName;
	SourceMap m_sources;
	std::size_t m_lineNumber = 0;
	std::map<std::string, std::string> m_settings;
	std::map<std::string, std::string> m_unreadSettings; ///< tracks which settings are left unread
};

template <typename E>
E TestCaseReader::enumSetting(std::string const& _name, std::map<std::string, E> const& _choices, std::string const& _defaultChoice)
{
	soltestAssert(_choices.count(_defaultChoice) > 0, "");

	std::string value = stringSetting(_name, _defaultChoice);

	if (_choices.count(value) == 0)
		BOOST_THROW_EXCEPTION(std::runtime_error(
			"Invalid Enum value: " + value + ". Available choices: " + util::joinHumanReadable(_choices | ranges::views::keys) + "."
		));

	return _choices.at(value);
}

}
