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
#include <libyul/AsmJsonConverter.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/YulString.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/Suite.h>

#include <libsolutil/JSON.h>

#include <cassert>
#include <memory>

using namespace std;
using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::phaser;

namespace solidity::phaser
{

ostream& operator<<(ostream& _stream, Program const& _program);

}

Program::Program(Program const& program):
	m_ast(make_unique<Block>(get<Block>(ASTCopier{}(*program.m_ast)))),
	m_dialect{program.m_dialect},
	m_nameDispenser(program.m_nameDispenser)
{
}

Program Program::load(CharStream& _sourceCode)
{
	// ASSUMPTION: parseSource() rewinds the stream on its own
	Dialect const& dialect = EVMDialect::strictAssemblyForEVMObjects(EVMVersion{});
	unique_ptr<Block> ast = parseSource(dialect, _sourceCode);
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
	m_ast = applyOptimisationSteps(m_dialect, m_nameDispenser, move(m_ast), _optimisationSteps);
}

ostream& phaser::operator<<(ostream& _stream, Program const& _program)
{
	return _stream << AsmPrinter()(*_program.m_ast);
}

string Program::toJson() const
{
	Json::Value serializedAst = AsmJsonConverter(0)(*m_ast);
	return jsonPrettyPrint(serializedAst);
}

unique_ptr<Block> Program::parseSource(Dialect const& _dialect, CharStream _source)
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	auto scanner = make_shared<Scanner>(move(_source));
	Parser parser(errorReporter, _dialect);

	unique_ptr<Block> ast = parser.parse(scanner, false);
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
	set<YulString> const externallyUsedIdentifiers = {};
	Disambiguator disambiguator(_dialect, _analysisInfo, externallyUsedIdentifiers);

	return make_unique<Block>(get<Block>(disambiguator(_ast)));
}

unique_ptr<Block> Program::applyOptimisationSteps(
	Dialect const& _dialect,
	NameDispenser& _nameDispenser,
	unique_ptr<Block> _ast,
	vector<string> const& _optimisationSteps
)
{
	// An empty set of reserved identifiers. It could be a constructor parameter but I don't
	// think it would be useful in this tool. Other tools (like yulopti) have it empty too.
	set<YulString> const externallyUsedIdentifiers = {};
	OptimiserStepContext context{_dialect, _nameDispenser, externallyUsedIdentifiers};

	for (string const& step: _optimisationSteps)
		OptimiserSuite::allSteps().at(step)->run(context, *_ast);

	return _ast;
}

size_t Program::computeCodeSize(Block const& _ast)
{
	return CodeSize::codeSizeIncludingFunctions(_ast);
}
