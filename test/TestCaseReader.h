// SPDX-License-Identifier: GPL-3.0

#include <fstream>
#include <map>
#include <string>

#pragma once

namespace solidity::frontend::test
{

/**
 * A map for registering source names that also contains the main source name in a test case.
 */
struct SourceMap
{
	std::map<std::string, std::string> sources;
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

	SourceMap const& sources() const { return m_sources; }
	std::string const& source() const;
	std::size_t lineNumber() const { return m_lineNumber; }
	std::map<std::string, std::string> const& settings() const { return m_settings; }
	std::ifstream& stream() { return m_file; }

	std::string simpleExpectations();

	bool boolSetting(std::string const& _name, bool _defaultValue);
	size_t sizetSetting(std::string const& _name, size_t _defaultValue);
	std::string stringSetting(std::string const& _name, std::string const& _defaultValue);

	void ensureAllSettingsRead() const;

private:
	std::pair<SourceMap, std::size_t> parseSourcesAndSettingsWithLineNumber(std::istream& _file);
	static std::string parseSimpleExpectations(std::istream& _file);

	std::ifstream m_file;
	SourceMap m_sources;
	std::size_t m_lineNumber = 0;
	std::map<std::string, std::string> m_settings;
	std::map<std::string, std::string> m_unreadSettings; ///< tracks which settings are left unread
};
}
