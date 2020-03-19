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

#include <fstream>
#include <map>
#include <string>

#pragma once

namespace solidity::frontend::test
{
/**
 * A reader for test case data file, which parses source, settings and (optionally) simple expectations.
 */
class TestCaseReader
{
public:
	TestCaseReader() = default;
	explicit TestCaseReader(std::string const& _filename);

	std::map<std::string, std::string> const& sources() { return m_sources; }
	std::string const& source();
	std::size_t lineNumber() { return m_lineNumber; }
	std::map<std::string, std::string> const& settings() { return m_settings; }
	std::ifstream& stream() { return m_file; }

	std::string simpleExpectations();

	bool boolSetting(std::string const& _name, bool _defaultValue);
	size_t sizetSetting(std::string const& _name, size_t _defaultValue);
	std::string stringSetting(std::string const& _name, std::string const& _defaultValue);

	void ensureAllSettingsRead() const;

private:
	std::pair<std::map<std::string, std::string>, std::size_t> parseSourcesAndSettingsWithLineNumber(std::istream& _file);
	static std::string parseSimpleExpectations(std::istream& _file);

	std::ifstream m_file;
	std::map<std::string, std::string> m_sources;
	std::size_t m_lineNumber = 0;
	std::map<std::string, std::string> m_settings;
	std::map<std::string, std::string> m_unreadSettings; ///< tracks which settings are left unread
};
}
