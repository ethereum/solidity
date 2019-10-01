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

#include <test/TestCase.h>

#include <libdevcore/StringUtils.h>

#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/range/adaptor/map.hpp>

#include <stdexcept>

#include <iostream>

using namespace dev;
using namespace solidity;
using namespace dev::solidity::test;
using namespace std;

void TestCase::printUpdatedSettings(ostream& _stream, const string& _linePrefix, const bool)
{
	if (m_validatedSettings.empty())
		return;

	_stream << _linePrefix << "// ====" << endl;
	for (auto const& setting: m_validatedSettings)
		_stream << _linePrefix << "// " << setting.first << ": " << setting.second << endl;
}

bool TestCase::isTestFilename(boost::filesystem::path const& _filename)
{
	string extension = _filename.extension().string();
	return (extension == ".sol" || extension == ".yul") &&
		   !boost::starts_with(_filename.string(), "~") &&
			!boost::starts_with(_filename.string(), ".");
}

bool TestCase::validateSettings(langutil::EVMVersion)
{
	if (!m_settings.empty())
		throw runtime_error(
			"Unknown setting(s): " +
			joinHumanReadable(m_settings | boost::adaptors::map_keys)
		);
	return true;
}

pair<map<string, string>, size_t> TestCase::parseSourcesAndSettingsWithLineNumbers(istream& _stream)
{
	map<string, string> sources;
	string currentSourceName;
	string currentSource;
	string line;
	size_t lineNumber = 1;
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
					throw runtime_error("Multiple definitions of test source \"" + currentSourceName + "\".");
			}
			else
				currentSource += line + "\n";
		}
		else if (boost::algorithm::starts_with(line, comment))
		{
			size_t colon = line.find(':');
			if (colon == string::npos)
				throw runtime_error(string("Expected \":\" inside setting."));
			string key = line.substr(comment.size(), colon - comment.size());
			string value = line.substr(colon + 1);
			boost::algorithm::trim(key);
			boost::algorithm::trim(value);
			m_settings[key] = value;
		}
		else
			throw runtime_error(string("Expected \"//\" or \"// ---\" to terminate settings and source."));
	}
	sources[currentSourceName] = currentSource;
	return {sources, lineNumber};
}

map<string, string> TestCase::parseSourcesAndSettings(istream& _stream)
{
	return get<0>(parseSourcesAndSettingsWithLineNumbers(_stream));
}

pair<string, size_t> TestCase::parseSourceAndSettingsWithLineNumbers(istream& _stream)
{
	auto [sourceMap, lineOffset] = parseSourcesAndSettingsWithLineNumbers(_stream);
	if (sourceMap.size() != 1)
		BOOST_THROW_EXCEPTION(runtime_error("Expected single source definition, but got multiple sources."));
	return {std::move(sourceMap.begin()->second), lineOffset};
}

string TestCase::parseSourceAndSettings(istream& _stream)
{
	return parseSourceAndSettingsWithLineNumbers(_stream).first;
}

string TestCase::parseSimpleExpectations(std::istream& _file)
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

void TestCase::expect(string::iterator& _it, string::iterator _end, string::value_type _c)
{
	if (_it == _end || *_it != _c)
		throw runtime_error(string("Invalid test expectation. Expected: \"") + _c + "\".");
	++_it;
}

bool EVMVersionRestrictedTestCase::validateSettings(langutil::EVMVersion _evmVersion)
{
	if (!m_settings.count("EVMVersion"))
		return true;

	string versionString = m_settings["EVMVersion"];
	m_validatedSettings["EVMVersion"] = versionString;
	m_settings.erase("EVMVersion");

	if (!TestCase::validateSettings(_evmVersion))
		return false;

	if (versionString.empty())
		return true;

	string comparator;
	size_t versionBegin = 0;
	for (auto character: versionString)
		if (!isalpha(character))
		{
			comparator += character;
			versionBegin++;
		}
		else
			break;

	versionString = versionString.substr(versionBegin);
	boost::optional<langutil::EVMVersion> version = langutil::EVMVersion::fromString(versionString);
	if (!version)
		throw runtime_error("Invalid EVM version: \"" + versionString + "\"");

	if (comparator == ">")
		return _evmVersion > version;
	else if (comparator == ">=")
		return _evmVersion >= version;
	else if (comparator == "<")
		return _evmVersion < version;
	else if (comparator == "<=")
		return _evmVersion <= version;
	else if (comparator == "=")
		return _evmVersion == version;
	else if (comparator == "!")
		return !(_evmVersion == version);
	else
		throw runtime_error("Invalid EVM comparator: \"" + comparator + "\"");
	return false; // not reached
}
