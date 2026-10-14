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

#include <test/libyul/ssa/ParserTest.h>

#include <libyul/backends/evm/ssa/io/Parser.h>

#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>

#include <libsolutil/AnsiColorized.h>
#include <libsolutil/CommonData.h>
#include <libsolutil/StringUtils.h>

#include <range/v3/algorithm/count.hpp>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::yul;
using namespace solidity::yul::ssa;
using namespace solidity::yul::ssa::io;
using namespace solidity::yul::test::ssa;
using namespace solidity::frontend::test;

std::unique_ptr<TestCase> ParserTest::create(Config const& _config)
{
	return std::make_unique<ParserTest>(_config.filename, _config.evmVersion);
}

ParserTest::ParserTest(std::string const& _filename, langutil::EVMVersion const _evmVersion):
	TestCase(_filename),
	m_evmVersion(_evmVersion)
{
	m_source = m_reader.source();
	m_expectation = m_reader.simpleExpectations();
}

TestCase::TestResult ParserTest::run(std::ostream& _stream, std::string const& _linePrefix, bool const _formatted)
{
	EVMDialect const& dialect = EVMDialect::strictAssemblyForEVMObjects(m_evmVersion);
	auto const& parsed = parse(m_source, dialect);
	if (!parsed)
	{
		ParseError const& error = parsed.error();
		SourceRange const& location = error.location;
		std::string_view const source = m_source;
		std::string_view const before = source.substr(0, location.begin);
		m_obtainedResult = fmt::format(
			"ParseError at line {}, {}: {}\n",
			1 + ranges::count(before, '\n'),
			escapeAndQuoteString(
				std::string(source.substr(location.begin, location.end - location.begin))
			),
			error.message
		);
		return checkResult(_stream, _linePrefix, _formatted);
	}
	m_obtainedResult = (*parsed)->print();

	auto const reparsed = parse(m_obtainedResult, dialect);
	soltestAssert(reparsed, "Printed graphs must parse.");
	if (
		std::string const reprinted = (*reparsed)->print();
		reprinted != m_obtainedResult
	)
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) <<
			_linePrefix <<
			"Printing is not stable under reparsing. Reparsed:" <<
			std::endl;
		printPrefixed(_stream, reprinted, _linePrefix + "  ");
		return TestResult::FatalError;
	}
	return checkResult(_stream, _linePrefix, _formatted);
}
