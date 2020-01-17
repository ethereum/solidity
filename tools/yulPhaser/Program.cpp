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

#include <tools/yulPhaser/Program.h>

#include <tools/yulPhaser/Exceptions.h>

#include <liblangutil/CharStream.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/Exceptions.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>
#include <libyul/AsmParser.h>
#include <libyul/YulString.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/Metrics.h>

#include <libyul/optimiser/Suite.h>

#include <libsolutil/CommonIO.h>

#include <boost/filesystem.hpp>

#include <cassert>
#include <memory>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::phaser;

set<YulString> const Program::s_externallyUsedIdentifiers = {};

Program Program::load(string const& _sourcePath)
{
	Dialect const& dialect = EVMDialect::strictAssemblyForEVMObjects(EVMVersion{});
	shared_ptr<Block> ast = parseSource(dialect, loadSource(_sourcePath));
	unique_ptr<AsmAnalysisInfo> analysisInfo = analyzeAST(dialect, *ast);

	Program program(
		dialect,
		disambiguateAST(dialect, *ast, *analysisInfo)
	);
	program.optimise({
		FunctionHoister::name,
		FunctionGrouper::name,
		ForLoopInitRewriter::name,
	});

	return program;
}

void Program::optimise(vector<string> const& _optimisationSteps)
{
	applyOptimisationSteps(m_optimiserStepContext, *m_ast, _optimisationSteps);
}

CharStream Program::loadSource(string const& _sourcePath)
{
	assertThrow(boost::filesystem::exists(_sourcePath), InvalidProgram, "Source file does not exist");

	string sourceCode = readFileAsString(_sourcePath);
	return CharStream(sourceCode, _sourcePath);
}

shared_ptr<Block> Program::parseSource(Dialect const& _dialect, CharStream _source)
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	auto scanner = make_shared<Scanner>(move(_source));
	Parser parser(errorReporter, _dialect);

	shared_ptr<Block> ast = parser.parse(scanner, false);
	assertThrow(ast != nullptr, InvalidProgram, "Error parsing source");
	assert(errorReporter.errors().empty());

	return ast;
}

unique_ptr<AsmAnalysisInfo> Program::analyzeAST(Dialect const& _dialect, Block const& _ast)
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	auto analysisInfo = make_unique<AsmAnalysisInfo>();
	AsmAnalyzer analyzer(*analysisInfo, errorReporter, _dialect);

	bool analysisSuccessful = analyzer.analyze(_ast);
	assertThrow(analysisSuccessful, InvalidProgram, "Error analyzing source");
	assert(errorReporter.errors().empty());

	return analysisInfo;
}

unique_ptr<Block> Program::disambiguateAST(
	Dialect const& _dialect,
	Block const& _ast,
	AsmAnalysisInfo const& _analysisInfo
)
{
	Disambiguator disambiguator(_dialect, _analysisInfo, s_externallyUsedIdentifiers);

	return make_unique<Block>(get<Block>(disambiguator(_ast)));
}

void Program::applyOptimisationSteps(
	OptimiserStepContext& _context,
	Block& _ast,
	vector<string> const& _optimisationSteps
)
{
	for (string const& step: _optimisationSteps)
		OptimiserSuite::allSteps().at(step)->run(_context, _ast);
}

size_t Program::computeCodeSize(Block const& _ast)
{
	return CodeSize::codeSizeIncludingFunctions(_ast);
}
