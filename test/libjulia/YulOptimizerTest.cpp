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

#include <test/libjulia/YulOptimizerTest.h>

#include <test/libsolidity/FormattedScope.h>

#include <test/Options.h>

#include <libjulia/optimiser/Disambiguator.h>
#include <libjulia/optimiser/CommonSubexpressionEliminator.h>
#include <libjulia/optimiser/NameCollector.h>
#include <libjulia/optimiser/ExpressionSplitter.h>
#include <libjulia/optimiser/FunctionGrouper.h>
#include <libjulia/optimiser/FunctionHoister.h>
#include <libjulia/optimiser/ExpressionInliner.h>
#include <libjulia/optimiser/FullInliner.h>
#include <libjulia/optimiser/MainFunction.h>

#include <libsolidity/parsing/Scanner.h>
#include <libsolidity/inlineasm/AsmPrinter.h>
#include <libsolidity/inlineasm/AsmParser.h>
#include <libsolidity/inlineasm/AsmAnalysis.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>
#include <libsolidity/interface/ErrorReporter.h>

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>

using namespace dev;
using namespace dev::julia;
using namespace dev::julia::test;
using namespace dev::solidity;
using namespace dev::solidity::test;
using namespace std;

YulOptimizerTest::YulOptimizerTest(string const& _filename)
{
	boost::filesystem::path path(_filename);

	if (path.empty() || std::next(path.begin()) == path.end() || std::next(std::next(path.begin())) == path.end())
		BOOST_THROW_EXCEPTION(runtime_error("Filename path has to contain a directory: \"" + _filename + "\"."));
	m_optimizerStep = std::prev(std::prev(path.end()))->string();

	ifstream file(_filename);
	if (!file)
		BOOST_THROW_EXCEPTION(runtime_error("Cannot open test case: \"" + _filename + "\"."));
	file.exceptions(ios::badbit);

	string line;
	while (getline(file, line))
	{
		if (boost::algorithm::starts_with(line, "// ----"))
			break;
		if (m_source.empty() && boost::algorithm::starts_with(line, "// yul"))
			m_yul = true;
		m_source += line + "\n";
	}
	while (getline(file, line))
		if (boost::algorithm::starts_with(line, "// "))
			m_expectation += line.substr(3) + "\n";
		else
			m_expectation += line + "\n";
}

bool YulOptimizerTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	assembly::AsmPrinter printer{m_yul};
	shared_ptr<Block> ast;
	shared_ptr<assembly::AsmAnalysisInfo> analysisInfo;
	if (!parse(_stream, _linePrefix, _formatted))
		return false;

	if (m_optimizerStep == "disambiguator")
		disambiguate();
	else if (m_optimizerStep == "commonSubexpressionEliminator")
	{
		disambiguate();
		(CommonSubexpressionEliminator{})(*m_ast);
	}
	else if (m_optimizerStep == "expressionSplitter")
	{
		NameDispenser nameDispenser;
		nameDispenser.m_usedNames = NameCollector(*m_ast).names();
		ExpressionSplitter{nameDispenser}(*m_ast);
	}
	else if (m_optimizerStep == "functionGrouper")
	{
		disambiguate();
		(FunctionGrouper{})(*m_ast);
	}
	else if (m_optimizerStep == "functionHoister")
	{
		disambiguate();
		(FunctionHoister{})(*m_ast);
	}
	else if (m_optimizerStep == "expressionInliner")
	{
		disambiguate();
		ExpressionInliner(*m_ast).run();
	}
	else if (m_optimizerStep == "fullInliner")
	{
		disambiguate();
		(FunctionHoister{})(*m_ast);
		(FunctionGrouper{})(*m_ast);
		FullInliner(*m_ast).run();
	}
	else if (m_optimizerStep == "mainFunction")
	{
		disambiguate();
		(FunctionGrouper{})(*m_ast);
		(MainFunction{})(*m_ast);
	}
	else
	{
		FormattedScope(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Invalid optimizer step: " << m_optimizerStep << endl;
		return false;
	}

	m_obtainedResult = m_optimizerStep + "\n" + printer(*m_ast) + "\n";

	if (m_expectation != m_obtainedResult)
	{
		string nextIndentLevel = _linePrefix + "  ";
		FormattedScope(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Expected result:" << endl;
		// TODO could compute a simple diff with highlighted lines
		printIndented(_stream, m_expectation, nextIndentLevel);
		FormattedScope(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Obtained result:" << endl;
		printIndented(_stream, m_obtainedResult, nextIndentLevel);
		return false;
	}
	return true;
}

void YulOptimizerTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	printIndented(_stream, m_source, _linePrefix);
}

void YulOptimizerTest::printUpdatedExpectations(ostream& _stream, string const& _linePrefix) const
{
	printIndented(_stream, m_obtainedResult, _linePrefix);
}

void YulOptimizerTest::printIndented(ostream& _stream, string const& _output, string const& _linePrefix) const
{
	stringstream output(_output);
	string line;
	while (getline(output, line))
		_stream << _linePrefix << line << endl;
}

bool YulOptimizerTest::parse(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	assembly::AsmFlavour flavour = m_yul ? assembly::AsmFlavour::Yul : assembly::AsmFlavour::Strict;
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	shared_ptr<Scanner> scanner = make_shared<Scanner>(CharStream(m_source), "");
	m_ast = assembly::Parser(errorReporter, flavour).parse(scanner, false);
	if (!m_ast || !errorReporter.errors().empty())
	{
		FormattedScope(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		printErrors(_stream, errorReporter.errors(), *scanner);
		return false;
	}
	m_analysisInfo = make_shared<assembly::AsmAnalysisInfo>();
	assembly::AsmAnalyzer analyzer(
		*m_analysisInfo,
		errorReporter,
		dev::test::Options::get().evmVersion(),
		boost::none,
		flavour
	);
	if (!analyzer.analyze(*m_ast) || !errorReporter.errors().empty())
	{
		FormattedScope(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error analyzing source." << endl;
		printErrors(_stream, errorReporter.errors(), *scanner);
		return false;
	}
	return true;
}

void YulOptimizerTest::disambiguate()
{
	*m_ast = boost::get<Block>(Disambiguator(*m_analysisInfo)(*m_ast));
	m_analysisInfo.reset();
}

void YulOptimizerTest::printErrors(ostream& _stream, ErrorList const& _errors, Scanner const& _scanner)
{
	SourceReferenceFormatter formatter(_stream, [&](string const&) -> Scanner const& { return _scanner; });

	for (auto const& error: _errors)
		formatter.printExceptionInformation(
			*error,
			(error->type() == Error::Type::Warning) ? "Warning" : "Error"
		);
}
