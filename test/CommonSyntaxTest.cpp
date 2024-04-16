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

#include <test/CommonSyntaxTest.h>
#include <test/Common.h>
#include <test/TestCase.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/CommonIO.h>
#include <libsolutil/StringUtils.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/throw_exception.hpp>

#include <fstream>
#include <memory>
#include <stdexcept>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::util::formatting;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;
using namespace solidity::test;
using namespace boost::unit_test;
namespace fs = boost::filesystem;

namespace
{

int parseUnsignedInteger(std::string::iterator& _it, std::string::iterator _end)
{
	if (_it == _end || !util::isDigit(*_it))
		BOOST_THROW_EXCEPTION(std::runtime_error("Invalid test expectation. Source location expected."));
	int result = 0;
	while (_it != _end && util::isDigit(*_it))
	{
		result *= 10;
		result += *_it - '0';
		++_it;
	}
	return result;
}

}

CommonSyntaxTest::CommonSyntaxTest(std::string const& _filename, langutil::EVMVersion _evmVersion):
	EVMVersionRestrictedTestCase(_filename),
	m_sources(m_reader.sources()),
	m_expectations(parseExpectations(m_reader.stream())),
	m_evmVersion(_evmVersion)
{
}

TestCase::TestResult CommonSyntaxTest::run(std::ostream& _stream, std::string const& _linePrefix, bool _formatted)
{
	parseCustomExpectations(m_reader.stream());
	parseAndAnalyze();

	return conclude(_stream, _linePrefix, _formatted);
}

TestCase::TestResult CommonSyntaxTest::conclude(std::ostream& _stream, std::string const& _linePrefix, bool _formatted)
{
	if (expectationsMatch())
		return TestResult::Success;

	printExpectationAndError(_stream, _linePrefix, _formatted);
	return TestResult::Failure;
}

void CommonSyntaxTest::printExpectationAndError(std::ostream& _stream, std::string const& _linePrefix, bool _formatted)
{
	std::string nextIndentLevel = _linePrefix + "  ";
	util::AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Expected result:" << std::endl;
	printExpectedResult(_stream, nextIndentLevel, _formatted);
	util::AnsiColorized(_stream, _formatted, {BOLD, CYAN}) << _linePrefix << "Obtained result:" << std::endl;
	printObtainedResult(_stream, nextIndentLevel, _formatted);
}

void CommonSyntaxTest::printSource(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const
{
	if (m_sources.sources.empty())
		return;

	assert(m_sources.externalSources.empty());
	bool outputSourceNames = (m_sources.sources.size() != 1 || !m_sources.sources.begin()->first.empty());

	for (auto const& [name, source]: m_sources.sources)
		if (_formatted)
		{
			if (source.empty())
				continue;

			if (outputSourceNames)
				_stream << _linePrefix << util::formatting::CYAN << "==== Source: " << name << " ====" << util::formatting::RESET << std::endl;
			std::vector<char const*> sourceFormatting(source.length(), util::formatting::RESET);
			for (auto const& error: m_errorList)
				if (error.sourceName == name && error.locationStart >= 0 && error.locationEnd >= 0)
				{
					assert(static_cast<size_t>(error.locationStart) <= source.length());
					assert(static_cast<size_t>(error.locationEnd) <= source.length());
					for (int i = error.locationStart; i < error.locationEnd; i++)
					{
						char const*& cellFormat = sourceFormatting[static_cast<size_t>(i)];
						char const* infoBgColor = SourceReferenceFormatter::errorHighlightColor(Error::Severity::Info);

						if (
							(error.type != Error::Type::Warning && error.type != Error::Type::Info) ||
							(error.type == Error::Type::Warning && (cellFormat == RESET || cellFormat == infoBgColor)) ||
							(error.type == Error::Type::Info && cellFormat == RESET)
						)
							cellFormat = SourceReferenceFormatter::errorHighlightColor(Error::errorSeverity(error.type));
					}
				}

			_stream << _linePrefix << sourceFormatting.front() << source.front();
			for (size_t i = 1; i < source.length(); i++)
			{
				if (sourceFormatting[i] != sourceFormatting[i - 1])
					_stream << sourceFormatting[i];
				if (source[i] != '\n')
					_stream << source[i];
				else
				{
					_stream << util::formatting::RESET << std::endl;
					if (i + 1 < source.length())
						_stream << _linePrefix << sourceFormatting[i];
				}
			}
			_stream << util::formatting::RESET;
		}
		else
		{
			if (outputSourceNames)
				printPrefixed(_stream, "==== Source: " + name + " ====", _linePrefix);
			printPrefixed(_stream, source, _linePrefix);
		}
}

void CommonSyntaxTest::parseCustomExpectations(std::istream& _stream)
{
	std::string remainingExpectations = boost::trim_copy(readUntilEnd(_stream));
	soltestAssert(
		remainingExpectations.empty(),
		"Found custom expectations not supported by the test case:\n" + remainingExpectations
	);
}

bool CommonSyntaxTest::expectationsMatch()
{
	return m_expectations == m_errorList;
}

void CommonSyntaxTest::printExpectedResult(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const
{
	printErrorList(_stream, m_expectations, _linePrefix, _formatted);
}

void CommonSyntaxTest::printObtainedResult(std::ostream& _stream, std::string const& _linePrefix, bool _formatted) const
{
	printErrorList(_stream, m_errorList, _linePrefix, _formatted);
}

void CommonSyntaxTest::printErrorList(
	std::ostream& _stream,
	std::vector<SyntaxTestError> const& _errorList,
	std::string const& _linePrefix,
	bool _formatted
)
{
	if (_errorList.empty())
	{
		if (_formatted)
			util::AnsiColorized(_stream, _formatted, {BOLD, GREEN}) << _linePrefix << "Success" << std::endl;
	}
	else
		for (auto const& error: _errorList)
		{
			{
				util::AnsiColorized scope(
					_stream,
					_formatted,
					{BOLD, SourceReferenceFormatter::errorTextColor(Error::errorSeverity(error.type))}
				);
				_stream << _linePrefix << Error::formatErrorType(error.type);
				if (error.errorId.has_value())
					_stream << ' ' << error.errorId->error;
				_stream << ": ";
			}
			if (!error.sourceName.empty() || error.locationStart >= 0 || error.locationEnd >= 0)
			{
				_stream << "(";
				if (!error.sourceName.empty())
					_stream << error.sourceName << ":";
				if (error.locationStart >= 0)
					_stream << error.locationStart;
				_stream << "-";
				if (error.locationEnd >= 0)
					_stream << error.locationEnd;
				_stream << "): ";
			}
			_stream << error.message << std::endl;
		}
}

std::string CommonSyntaxTest::errorMessage(util::Exception const& _e)
{
	if (_e.comment() && !_e.comment()->empty())
		return boost::replace_all_copy(*_e.comment(), "\n", "\\n");
	else
		return "NONE";
}

std::vector<SyntaxTestError> CommonSyntaxTest::parseExpectations(std::istream& _stream)
{
	static std::string const customExpectationsDelimiter("// ----");

	std::vector<SyntaxTestError> expectations;
	std::string line;
	while (std::getline(_stream, line))
	{
		auto it = line.begin();

		// Anything below the delimiter is left up to the derived class to process in a custom way.
		// The delimiter is optional and identical to the one that starts error expectations in
		// TestCaseReader::parseSourcesAndSettingsWithLineNumber().
		if (boost::algorithm::starts_with(line, customExpectationsDelimiter))
			break;

		skipSlashes(it, line.end());
		skipWhitespace(it, line.end());

		if (it == line.end()) continue;

		auto typeBegin = it;
		while (it != line.end() && isalpha(*it, std::locale::classic()))
			++it;

		std::string errorTypeStr(typeBegin, it);
		std::optional<Error::Type> errorType = Error::parseErrorType(errorTypeStr);
		if (!errorType.has_value())
			BOOST_THROW_EXCEPTION(std::runtime_error("Invalid error type: " + errorTypeStr));

		skipWhitespace(it, line.end());

		std::optional<ErrorId> errorId;
		if (it != line.end() && util::isDigit(*it))
			errorId = ErrorId{static_cast<unsigned long long>(parseUnsignedInteger(it, line.end()))};

		expect(it, line.end(), ':');
		skipWhitespace(it, line.end());

		int locationStart = -1;
		int locationEnd = -1;
		std::string sourceName;

		if (it != line.end() && *it == '(')
		{
			++it;
			if (it != line.end() && !util::isDigit(*it))
			{
				auto sourceNameStart = it;
				while (it != line.end() && *it != ':')
					++it;
				sourceName = std::string(sourceNameStart, it);
				expect(it, line.end(), ':');
			}
			locationStart = parseUnsignedInteger(it, line.end());
			expect(it, line.end(), '-');
			locationEnd = parseUnsignedInteger(it, line.end());
			expect(it, line.end(), ')');
			expect(it, line.end(), ':');
		}

		skipWhitespace(it, line.end());

		std::string errorMessage(it, line.end());
		expectations.emplace_back(SyntaxTestError{
			errorType.value(),
			std::move(errorId),
			std::move(errorMessage),
			std::move(sourceName),
			locationStart,
			locationEnd
		});
	}
	return expectations;
}
