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

#include <test/libyul/YulOptimizerTest.h>

#include <test/Options.h>

#include <libyul/optimiser/BlockFlattener.h>
#include <libyul/optimiser/VarDeclInitializer.h>
#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/optimiser/DeadCodeEliminator.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/CommonSubexpressionEliminator.h>
#include <libyul/optimiser/NameCollector.h>
#include <libyul/optimiser/EquivalentFunctionCombiner.h>
#include <libyul/optimiser/ExpressionSplitter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/ExpressionInliner.h>
#include <libyul/optimiser/FullInliner.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/MainFunction.h>
#include <libyul/optimiser/Rematerialiser.h>
#include <libyul/optimiser/ExpressionSimplifier.h>
#include <libyul/optimiser/UnusedPruner.h>
#include <libyul/optimiser/ExpressionJoiner.h>
#include <libyul/optimiser/SSAReverser.h>
#include <libyul/optimiser/SSATransform.h>
#include <libyul/optimiser/RedundantAssignEliminator.h>
#include <libyul/optimiser/StructuralSimplifier.h>
#include <libyul/optimiser/StackCompressor.h>
#include <libyul/optimiser/Suite.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AssemblyStack.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Scanner.h>

#include <libdevcore/AnsiColorized.h>

#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>

using namespace dev;
using namespace langutil;
using namespace yul;
using namespace yul::test;
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
	soltestAssert(file, "Cannot open test contract: \"" + _filename + "\".");
	file.exceptions(ios::badbit);

	m_source = parseSourceAndSettings(file);
	if (m_settings.count("yul"))
	{
		m_yul = true;
		m_validatedSettings["yul"] = "true";
		m_settings.erase("yul");
	}
	if (m_settings.count("step"))
	{
		m_validatedSettings["step"] = m_settings["step"];
		m_settings.erase("step");
	}

	string line;
	while (getline(file, line))
		if (boost::algorithm::starts_with(line, "// "))
			m_expectation += line.substr(3) + "\n";
		else
			m_expectation += line + "\n";
}

bool YulOptimizerTest::run(ostream& _stream, string const& _linePrefix, bool const _formatted)
{
	if (!parse(_stream, _linePrefix, _formatted))
		return false;

	if (m_optimizerStep == "disambiguator")
		disambiguate();
	else if (m_optimizerStep == "blockFlattener")
	{
		disambiguate();
		BlockFlattener{}(*m_ast);
	}
	else if (m_optimizerStep == "varDeclInitializer")
		VarDeclInitializer{}(*m_ast);
	else if (m_optimizerStep == "varNameCleaner")
		VarNameCleaner{*m_ast, *m_dialect}(*m_ast);
	else if (m_optimizerStep == "forLoopInitRewriter")
	{
		disambiguate();
		ForLoopInitRewriter{}(*m_ast);
	}
	else if (m_optimizerStep == "commonSubexpressionEliminator")
	{
		disambiguate();
		(CommonSubexpressionEliminator{*m_dialect})(*m_ast);
	}
	else if (m_optimizerStep == "expressionSplitter")
	{
		NameDispenser nameDispenser{*m_dialect, *m_ast};
		ExpressionSplitter{*m_dialect, nameDispenser}(*m_ast);
	}
	else if (m_optimizerStep == "expressionJoiner")
	{
		disambiguate();
		ExpressionJoiner::run(*m_ast);
	}
	else if (m_optimizerStep == "splitJoin")
	{
		disambiguate();
		NameDispenser nameDispenser{*m_dialect, *m_ast};
		ExpressionSplitter{*m_dialect, nameDispenser}(*m_ast);
		ExpressionJoiner::run(*m_ast);
		ExpressionJoiner::run(*m_ast);
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
		ExpressionInliner(*m_dialect, *m_ast).run();
	}
	else if (m_optimizerStep == "fullInliner")
	{
		disambiguate();
		(FunctionHoister{})(*m_ast);
		(FunctionGrouper{})(*m_ast);
		NameDispenser nameDispenser{*m_dialect, *m_ast};
		ExpressionSplitter{*m_dialect, nameDispenser}(*m_ast);
		FullInliner(*m_ast, nameDispenser).run();
		ExpressionJoiner::run(*m_ast);
	}
	else if (m_optimizerStep == "mainFunction")
	{
		disambiguate();
		(FunctionGrouper{})(*m_ast);
		(MainFunction{})(*m_ast);
	}
	else if (m_optimizerStep == "rematerialiser")
	{
		disambiguate();
		Rematerialiser::run(*m_dialect, *m_ast);
	}
	else if (m_optimizerStep == "expressionSimplifier")
	{
		disambiguate();
		ExpressionSimplifier::run(*m_dialect, *m_ast);
	}
	else if (m_optimizerStep == "fullSimplify")
	{
		disambiguate();
		NameDispenser nameDispenser{*m_dialect, *m_ast};
		ExpressionSplitter{*m_dialect, nameDispenser}(*m_ast);
		CommonSubexpressionEliminator{*m_dialect}(*m_ast);
		ExpressionSimplifier::run(*m_dialect, *m_ast);
		UnusedPruner::runUntilStabilised(*m_dialect, *m_ast);
		DeadCodeEliminator{}(*m_ast);
		ExpressionJoiner::run(*m_ast);
		ExpressionJoiner::run(*m_ast);
	}
	else if (m_optimizerStep == "unusedPruner")
	{
		disambiguate();
		UnusedPruner::runUntilStabilised(*m_dialect, *m_ast);
	}
	else if (m_optimizerStep == "deadCodeEliminator")
	{
		disambiguate();
		DeadCodeEliminator{}(*m_ast);
	}
	else if (m_optimizerStep == "ssaTransform")
	{
		disambiguate();
		NameDispenser nameDispenser{*m_dialect, *m_ast};
		SSATransform::run(*m_ast, nameDispenser);
	}
	else if (m_optimizerStep == "redundantAssignEliminator")
	{
		disambiguate();
		RedundantAssignEliminator::run(*m_dialect, *m_ast);
	}
	else if (m_optimizerStep == "ssaPlusCleanup")
	{
		disambiguate();
		NameDispenser nameDispenser{*m_dialect, *m_ast};
		SSATransform::run(*m_ast, nameDispenser);
		RedundantAssignEliminator::run(*m_dialect, *m_ast);
	}
	else if (m_optimizerStep == "structuralSimplifier")
	{
		disambiguate();
		StructuralSimplifier{*m_dialect}(*m_ast);
	}
	else if (m_optimizerStep == "equivalentFunctionCombiner")
	{
		disambiguate();
		EquivalentFunctionCombiner::run(*m_ast);
	}
	else if (m_optimizerStep == "ssaReverser")
	{
		disambiguate();
		SSAReverser::run(*m_ast);
	}
	else if (m_optimizerStep == "ssaAndBack")
	{
		disambiguate();
		// apply SSA
		NameDispenser nameDispenser{*m_dialect, *m_ast};
		SSATransform::run(*m_ast, nameDispenser);
		RedundantAssignEliminator::run(*m_dialect, *m_ast);
		// reverse SSA
		SSAReverser::run(*m_ast);
		CommonSubexpressionEliminator{*m_dialect}(*m_ast);
		UnusedPruner::runUntilStabilised(*m_dialect, *m_ast);
	}
	else if (m_optimizerStep == "stackCompressor")
	{
		disambiguate();
		(FunctionGrouper{})(*m_ast);
		size_t maxIterations = 16;
		StackCompressor::run(m_dialect, *m_ast, true, maxIterations);
		(BlockFlattener{})(*m_ast);
	}
	else if (m_optimizerStep == "fullSuite")
		OptimiserSuite::run(m_dialect, *m_ast, *m_analysisInfo, true);
	else
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Invalid optimizer step: " << m_optimizerStep << endl;
		return false;
	}

	m_obtainedResult = AsmPrinter{m_yul}(*m_ast) + "\n";

	bool success = true;
	if (m_optimizerStep != m_validatedSettings["step"])
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) <<
			_linePrefix <<
			"Invalid optimizer step. Given: \"" <<
			m_validatedSettings["step"] <<
			"\", should be: \"" <<
			m_optimizerStep <<
			"\"." <<
			endl;
		success = false;
	}
	if (m_expectation != m_obtainedResult)
	{
		string nextIndentLevel = _linePrefix + "  ";
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Expected result:" << endl;
		// TODO could compute a simple diff with highlighted lines
		printIndented(_stream, m_expectation, nextIndentLevel);
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::CYAN}) << _linePrefix << "Obtained result:" << endl;
		printIndented(_stream, m_obtainedResult, nextIndentLevel);
		success = false;
	}
	return success;
}

void YulOptimizerTest::printSource(ostream& _stream, string const& _linePrefix, bool const) const
{
	printIndented(_stream, m_source, _linePrefix);
}

void YulOptimizerTest::printUpdatedSettings(ostream& _stream, const string& _linePrefix, const bool _formatted)
{
	m_validatedSettings["step"] = m_optimizerStep;
	EVMVersionRestrictedTestCase::printUpdatedSettings(_stream, _linePrefix, _formatted);
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
	AssemblyStack stack(
		dev::test::Options::get().evmVersion(),
		m_yul ? AssemblyStack::Language::Yul : AssemblyStack::Language::StrictAssembly,
		dev::solidity::OptimiserSettings::none()
	);
	if (!stack.parseAndAnalyze("", m_source) || !stack.errors().empty())
	{
		AnsiColorized(_stream, _formatted, {formatting::BOLD, formatting::RED}) << _linePrefix << "Error parsing source." << endl;
		printErrors(_stream, stack.errors());
		return false;
	}
	m_dialect = m_yul ? Dialect::yul() : EVMDialect::strictAssemblyForEVMObjects(dev::test::Options::get().evmVersion());
	m_ast = stack.parserResult()->code;
	m_analysisInfo = stack.parserResult()->analysisInfo;
	return true;
}

void YulOptimizerTest::disambiguate()
{
	*m_ast = boost::get<Block>(Disambiguator(*m_dialect, *m_analysisInfo)(*m_ast));
	m_analysisInfo.reset();
}

void YulOptimizerTest::printErrors(ostream& _stream, ErrorList const& _errors)
{
	SourceReferenceFormatter formatter(_stream);

	for (auto const& error: _errors)
		formatter.printExceptionInformation(
			*error,
			(error->type() == Error::Type::Warning) ? "Warning" : "Error"
		);
}
