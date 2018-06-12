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

#include <test/libsolidity/SyntaxTest.h>
#include <test/Options.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/throw_exception.hpp>
#include <cctype>
#include <fstream>
#include <memory>
#include <stdexcept>

using namespace dev;
using namespace solidity;
using namespace dev::solidity::test;
using namespace dev::solidity::test::formatting;
using namespace std;
namespace fs = boost::filesystem;
using namespace boost::unit_test;

template<typename IteratorType>
void skipWhitespace(IteratorType& _it, IteratorType _end)
{
	while (_it != _end && isspace(*_it))
		++_it;
}

template<typename IteratorType>
void skipSlashes(IteratorType& _it, IteratorType _end)
{
	while (_it != _end && *_it == '/')
		++_it;
}

void expect(string::iterator& _it, string::iterator _end, string::value_type _c)
{
	if (_it == _end || *_it != _c)
		throw runtime_error(string("Invalid test expectation. Expected: \"") + _c + "\".");
	++_it;
}

int parseUnsignedInteger(string::iterator &_it, string::iterator _end)
{
	if (_it == _end || !isdigit(*_it))
		throw runtime_error("Invalid test expectation. Source location expected.");
	int result = 0;
	while (_it != _end && isdigit(*_it))
	{
		result *= 10;
		result += *_it - '0';
		++_it;
	}
	return result;
}

SyntaxTest::SyntaxTest(string const& _filename)
{
	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test contract: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	m_source = parseSource(file);
	m_expectations = parseExpectations(file);
}

bool SyntaxTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	string const versionPragma = "pragma solidity >=0.0;\n";
	m_compiler.reset();
	m_compiler.addSource("", versionPragma + m_source);
	m_compiler.setEVMVersion(dev::test::Options::get().evmVersion());

	if (m_compiler.parse())
		m_compiler.analyze();

	for (auto const& currentError: filterErrors(m_compiler.errors(), true))
	{
		int locationStart = -1, locationEnd = -1;
		if (auto location = boost::get_error_info<errinfo_sourceLocation>(*currentError))
		{
			// ignore the version pragma inserted by the testing tool when calculating locations.
			if (location->start >= static_cast<int>(versionPragma.size()))
				locationStart = location->start - versionPragma.size();
			if (location->end >= static_cast<int>(versionPragma.size()))
				locationEnd = location->end - versionPragma.size();
		}
		m_errorList.emplace_back(SyntaxTestError{
			currentError->typeName(),
			errorMessage(*currentError),
			locationStart,
			locationEnd
		});
	}

	if (m_expectations != m_errorList)
	{
		string nextIndentLevel = _linePrefix + "  ";
		FormattedScope(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << endl;
		printErrorList(_stream, m_expectations, nextIndentLevel, _formatted);
		FormattedScope(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result:" << endl;
		printErrorList(_stream, m_errorList, nextIndentLevel, _formatted);
		return false;
	}
	return true;
}

void SyntaxTest::printErrorList(
	ostream& _stream,
	vector<SyntaxTestError> const& _errorList,
	string const& _linePrefix,
	bool const _formatted
)
{
	if (_errorList.empty())
		FormattedScope(_stream, _formatted, {BOLD, GREEN}) << _linePrefix << "Success" << endl;
	else
		for (auto const& error: _errorList)
		{
			{
				FormattedScope scope(_stream, _formatted, {BOLD, (error.type == "Warning") ? YELLOW : RED});
				_stream << _linePrefix;
				_stream << error.type << ": ";
			}
			if (error.locationStart >= 0 || error.locationEnd >= 0)
			{
				_stream << "(";
				if (error.locationStart >= 0)
					_stream << error.locationStart;
				_stream << "-";
				if (error.locationEnd >= 0)
					_stream << error.locationEnd;
				_stream << "): ";
			}
			_stream << error.message << endl;
		}
}

string SyntaxTest::errorMessage(Exception const& _e)
{
	if (_e.comment() && !_e.comment()->empty())
		return boost::replace_all_copy(*_e.comment(), "\n", "\\n");
	else
		return "NONE";
}

string SyntaxTest::parseSource(istream& _stream)
{
	string source;
	string line;
	string const delimiter("// ----");
	while (getline(_stream, line))
		if (boost::algorithm::starts_with(line, delimiter))
			break;
		else
			source += line + "\n";
	return source;
}

vector<SyntaxTestError> SyntaxTest::parseExpectations(istream& _stream)
{
	vector<SyntaxTestError> expectations;
	string line;
	while (getline(_stream, line))
	{
		auto it = line.begin();

		skipSlashes(it, line.end());
		skipWhitespace(it, line.end());

		if (it == line.end()) continue;

		auto typeBegin = it;
		while (it != line.end() && *it != ':')
			++it;
		string errorType(typeBegin, it);

		// skip colon
		if (it != line.end()) it++;

		skipWhitespace(it, line.end());

		int locationStart = -1;
		int locationEnd = -1;

		if (it != line.end() && *it == '(')
		{
			++it;
			locationStart = parseUnsignedInteger(it, line.end());
			expect(it, line.end(), '-');
			locationEnd = parseUnsignedInteger(it, line.end());
			expect(it, line.end(), ')');
			expect(it, line.end(), ':');
		}

		skipWhitespace(it, line.end());

		string errorMessage(it, line.end());
		expectations.emplace_back(SyntaxTestError{
			move(errorType),
			move(errorMessage),
			locationStart,
			locationEnd
		});
	}
	return expectations;
}

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

bool SyntaxTest::isTestFilename(boost::filesystem::path const& _filename)
{
	return _filename.extension().string() == ".sol" &&
		!boost::starts_with(_filename.string(), "~") &&
		!boost::starts_with(_filename.string(), ".");
}

int SyntaxTest::registerTests(
	boost::unit_test::test_suite& _suite,
	boost::filesystem::path const& _basepath,
	boost::filesystem::path const& _path
)
{
	int numTestsAdded = 0;
	fs::path fullpath = _basepath / _path;
	if (fs::is_directory(fullpath))
	{
		test_suite* sub_suite = BOOST_TEST_SUITE(_path.filename().string());
		for (auto const& entry: boost::iterator_range<fs::directory_iterator>(
			fs::directory_iterator(fullpath),
			fs::directory_iterator()
		))
			if (fs::is_directory(entry.path()) || isTestFilename(entry.path().filename()))
				numTestsAdded += registerTests(*sub_suite, _basepath, _path / entry.path().filename());
		_suite.add(sub_suite);
	}
	else
	{
		static vector<unique_ptr<string>> filenames;

		filenames.emplace_back(new string(_path.string()));
		_suite.add(make_test_case(
			[fullpath]
			{
				BOOST_REQUIRE_NO_THROW({
					try
					{
						stringstream errorStream;
						if (!SyntaxTest(fullpath.string()).run(errorStream))
							BOOST_ERROR("Test expectation mismatch.\n" + errorStream.str());
					}
					catch (boost::exception const& _e)
					{
						BOOST_ERROR("Exception during syntax test: " << boost::diagnostic_information(_e));
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
