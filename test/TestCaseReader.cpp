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

#include <test/TestCaseReader.h>

#include <libsolutil/CommonIO.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace solidity::frontend::test;

namespace fs = boost::filesystem;

TestCaseReader::TestCaseReader(string const& _filename): m_fileStream(_filename), m_fileName(_filename)
{
	if (!m_fileStream)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open file: \"" + _filename + "\"."));
	m_fileStream.exceptions(ios::badbit);

	tie(m_sources, m_lineNumber) = parseSourcesAndSettingsWithLineNumber(m_fileStream);
	m_unreadSettings = m_settings;
}

TestCaseReader::TestCaseReader(istringstream const& _str)
{
	tie(m_sources, m_lineNumber) = parseSourcesAndSettingsWithLineNumber(
		static_cast<istream&>(const_cast<istringstream&>(_str))
	);
}

string const& TestCaseReader::source() const
{
	if (m_sources.sources.size() != 1)
		BOOST_THROW_EXCEPTION(runtime_error("Expected single source definition, but got multiple sources."));
	return m_sources.sources.at(m_sources.mainSourceFile);
}

string TestCaseReader::simpleExpectations()
{
	return parseSimpleExpectations(m_fileStream);
}

bool TestCaseReader::boolSetting(std::string const& _name, bool _defaultValue)
{
	if (m_settings.count(_name) == 0)
		return _defaultValue;

	m_unreadSettings.erase(_name);
	string value = m_settings.at(_name);
	if (value == "false")
		return false;
	if (value == "true")
		return true;

	BOOST_THROW_EXCEPTION(runtime_error("Invalid Boolean value: " + value + "."));
}

size_t TestCaseReader::sizetSetting(std::string const& _name, size_t _defaultValue)
{
	if (m_settings.count(_name) == 0)
		return _defaultValue;

	m_unreadSettings.erase(_name);

	static_assert(sizeof(unsigned long) <= sizeof(size_t));
	return stoul(m_settings.at(_name));
}

string TestCaseReader::stringSetting(string const& _name, string const& _defaultValue)
{
	if (m_settings.count(_name) == 0)
		return _defaultValue;

	m_unreadSettings.erase(_name);
	return m_settings.at(_name);
}

void TestCaseReader::ensureAllSettingsRead() const
{
	if (!m_unreadSettings.empty())
		BOOST_THROW_EXCEPTION(runtime_error(
			"Unknown setting(s): " +
			util::joinHumanReadable(m_unreadSettings | ranges::views::keys)
		));
}

pair<SourceMap, size_t> TestCaseReader::parseSourcesAndSettingsWithLineNumber(istream& _stream)
{
	map<string, string> sources;
	map<string, boost::filesystem::path> externalSources;
	string currentSourceName;
	string currentSource;
	string line;
	size_t lineNumber = 1;
	static string const externalSourceDelimiterStart("==== ExternalSource:");
	static string const sourceDelimiterStart("==== Source:");
	static string const sourceDelimiterEnd("====");
	static string const comment("// ");
	static string const settingsDelimiter("// ====");
	static string const delimiter("// ----");
	bool sourcePart = true;
	while (getline(_stream, line))
	{
		lineNumber++;

		if (boost::algorithm::starts_with(line, delimiter))
			break;
		else if (boost::algorithm::starts_with(line, settingsDelimiter))
			sourcePart = false;
		else if (sourcePart)
		{
			if (boost::algorithm::starts_with(line, sourceDelimiterStart) && boost::algorithm::ends_with(line, sourceDelimiterEnd))
			{
				if (!(currentSourceName.empty() && currentSource.empty()))
					sources[currentSourceName] = std::move(currentSource);
				currentSource = {};
				currentSourceName = boost::trim_copy(line.substr(
					sourceDelimiterStart.size(),
					line.size() - sourceDelimiterEnd.size() - sourceDelimiterStart.size()
				));
				if (sources.count(currentSourceName))
					BOOST_THROW_EXCEPTION(runtime_error("Multiple definitions of test source \"" + currentSourceName + "\"."));
			}
			else if (boost::algorithm::starts_with(line, externalSourceDelimiterStart) && boost::algorithm::ends_with(line, sourceDelimiterEnd))
			{
				string externalSourceString = boost::trim_copy(line.substr(
					externalSourceDelimiterStart.size(),
					line.size() - sourceDelimiterEnd.size() - externalSourceDelimiterStart.size()
				));

				string externalSourceName;
				size_t remappingPos = externalSourceString.find('=');
				// Does the external source define a remapping?
				if (remappingPos != string::npos)
				{
					externalSourceName = boost::trim_copy(externalSourceString.substr(0, remappingPos));
					externalSourceString = boost::trim_copy(externalSourceString.substr(remappingPos + 1));
				}
				else
					externalSourceName = externalSourceString;

				soltestAssert(!externalSourceName.empty(), "");
				fs::path externalSourceTarget(externalSourceString);
				fs::path testCaseParentDir = m_fileName.parent_path();
				if (!externalSourceTarget.is_relative() || !externalSourceTarget.root_path().empty())
					// NOTE: UNC paths (ones starting with // or \\) are considered relative by Boost
					// since they have an empty root directory (but non-empty root name).
					BOOST_THROW_EXCEPTION(runtime_error("External Source paths need to be relative to the location of the test case."));
				fs::path externalSourceFullPath = testCaseParentDir / externalSourceTarget;
				string externalSourceContent;
				if (!fs::exists(externalSourceFullPath))
					BOOST_THROW_EXCEPTION(runtime_error("External Source '" + externalSourceTarget.string() + "' not found."));
				else
					externalSourceContent = util::readFileAsString(externalSourceFullPath);

				if (sources.count(externalSourceName))
					BOOST_THROW_EXCEPTION(runtime_error("Multiple definitions of test source \"" + externalSourceName + "\"."));
				sources[externalSourceName] = externalSourceContent;
				externalSources[externalSourceName] = externalSourceTarget;
			}
			else
				currentSource += line + "\n";
		}
		else if (boost::algorithm::starts_with(line, comment))
		{
			size_t colon = line.find(':');
			if (colon == string::npos)
				BOOST_THROW_EXCEPTION(runtime_error(string("Expected \":\" inside setting.")));
			string key = line.substr(comment.size(), colon - comment.size());
			string value = line.substr(colon + 1);
			boost::algorithm::trim(key);
			boost::algorithm::trim(value);
			m_settings[key] = value;
		}
		else
			BOOST_THROW_EXCEPTION(runtime_error(string("Expected \"//\" or \"// ---\" to terminate settings and source.")));
	}
	// Register the last source as the main one
	sources[currentSourceName] = currentSource;
	return {{std::move(sources), std::move(externalSources), std::move(currentSourceName)}, lineNumber};
}

string TestCaseReader::parseSimpleExpectations(istream& _file)
{
	string result;
	string line;
	while (getline(_file, line))
		if (boost::algorithm::starts_with(line, "// "))
			result += line.substr(3) + "\n";
		else if (line == "//")
			result += "\n";
		else
			BOOST_THROW_EXCEPTION(runtime_error("Test expectations must start with \"// \"."));
	return result;
}
