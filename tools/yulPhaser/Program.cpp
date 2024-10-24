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

#include <tools/yulPhaser/Program.h>

#include <liblangutil/CharStream.h>
#include <liblangutil/ErrorReporter.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmJsonConverter.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/ObjectParser.h>
#include <libyul/YulName.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/optimiser/Disambiguator.h>
#include <libyul/optimiser/ForLoopInitRewriter.h>
#include <libyul/optimiser/FunctionGrouper.h>
#include <libyul/optimiser/FunctionHoister.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/optimiser/OptimiserStep.h>
#include <libyul/optimiser/Suite.h>

#include <libsolutil/JSON.h>

#include <libsolidity/interface/OptimiserSettings.h>

#include <cassert>
#include <memory>

using namespace solidity;
using namespace solidity::langutil;
using namespace solidity::yul;
using namespace solidity::util;
using namespace solidity::phaser;

namespace solidity::phaser
{

std::ostream& operator<<(std::ostream& _stream, Program const& _program);

}

Program::Program(Program const& program):
	m_ast(std::make_unique<AST>(std::get<Block>(ASTCopier{}(program.m_ast->root())))),
	m_dialect{program.m_dialect},
	m_nameDispenser(program.m_nameDispenser)
{
}

std::variant<Program, ErrorList> Program::load(CharStream& _sourceCode)
{
	// ASSUMPTION: parseSource() rewinds the stream on its own
	// TODO: Add support for EOF
	Dialect const& dialect = EVMDialect::strictAssemblyForEVMObjects(EVMVersion{}, std::nullopt);

	std::variant<std::unique_ptr<AST>, ErrorList> astOrErrors = parseObject(dialect, _sourceCode);
	if (std::holds_alternative<ErrorList>(astOrErrors))
		return std::get<ErrorList>(astOrErrors);

	std::variant<std::unique_ptr<AsmAnalysisInfo>, ErrorList> analysisInfoOrErrors = analyzeAST(
		dialect,
		*std::get<std::unique_ptr<AST>>(astOrErrors)
	);
	if (std::holds_alternative<ErrorList>(analysisInfoOrErrors))
		return std::get<ErrorList>(analysisInfoOrErrors);

	Program program(
		dialect,
		disambiguateAST(
			dialect,
			*std::get<std::unique_ptr<AST>>(astOrErrors),
			*std::get<std::unique_ptr<AsmAnalysisInfo>>(analysisInfoOrErrors)
		)
	);
	program.optimise({
		FunctionHoister::name,
		FunctionGrouper::name,
		ForLoopInitRewriter::name,
	});

	return program;
}

void Program::optimise(std::vector<std::string> const& _optimisationSteps)
{
	m_ast = applyOptimisationSteps(m_dialect, m_nameDispenser, std::move(m_ast), _optimisationSteps);
}

std::ostream& phaser::operator<<(std::ostream& _stream, Program const& _program)
{
	return _stream << AsmPrinter(_program.m_dialect)(_program.m_ast->root());
}

std::string Program::toJson() const
{
	Json serializedAst = AsmJsonConverter(m_dialect, 0)(m_ast->root());
	return jsonPrettyPrint(removeNullMembers(std::move(serializedAst)));
}

std::variant<std::unique_ptr<AST>, ErrorList> Program::parseObject(Dialect const& _dialect, CharStream _source)
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	auto scanner = std::make_shared<Scanner>(_source);

	ObjectParser parser(errorReporter, _dialect);
	std::shared_ptr<Object> object = parser.parse(scanner, false);
	if (object == nullptr || errorReporter.hasErrors())
		// NOTE: It's possible to get errors even if the returned object is non-null.
		// For example when there are errors in a nested object.
		return errors;

	Object* deployedObject = nullptr;
	if (object->subObjects.size() > 0)
		for (auto& subObject: object->subObjects)
			// solc --ir produces an object with a subobject of the same name as the outer object
			// but suffixed with  "_deployed".
			// The other object references the nested one which makes analysis fail. Below we try to
			// extract just the nested one for that reason. This is just a heuristic. If there's no
			// subobject with such a suffix we fall back to accepting the whole object as is.
			if (subObject != nullptr && subObject->name == object->name + "_deployed")
			{
				deployedObject = dynamic_cast<Object*>(subObject.get());
				if (deployedObject != nullptr)
					break;
			}
	Object* selectedObject = (deployedObject != nullptr ? deployedObject : object.get());

	// NOTE: I'm making a copy of the whole AST to get unique_ptr rather than shared_ptr.
	// This is a slight performance hit but it's much less than the parsing itself.
	// unique_ptr lets me be sure that two Program instances can never share the AST by mistake.
	// The public API of the class does not provide access to the smart pointer so it won't be hard
	// to switch to shared_ptr if the copying turns out to be an issue (though it would be better
	// to refactor ObjectParser and Object to use unique_ptr instead).
	auto astCopy = std::make_unique<AST>(std::get<Block>(ASTCopier{}(selectedObject->code()->root())));

	return std::variant<std::unique_ptr<AST>, ErrorList>(std::move(astCopy));
}

std::variant<std::unique_ptr<AsmAnalysisInfo>, ErrorList> Program::analyzeAST(Dialect const& _dialect, AST const& _ast)
{
	ErrorList errors;
	ErrorReporter errorReporter(errors);
	auto analysisInfo = std::make_unique<AsmAnalysisInfo>();
	AsmAnalyzer analyzer(*analysisInfo, errorReporter, _dialect);

	bool analysisSuccessful = analyzer.analyze(_ast.root());
	if (!analysisSuccessful)
		return errors;

	assert(!errorReporter.hasErrors());
	return std::variant<std::unique_ptr<AsmAnalysisInfo>, ErrorList>(std::move(analysisInfo));
}

std::unique_ptr<AST> Program::disambiguateAST(
	Dialect const& _dialect,
	AST const& _ast,
	AsmAnalysisInfo const& _analysisInfo
)
{
	std::set<YulName> const externallyUsedIdentifiers = {};
	Disambiguator disambiguator(_dialect, _analysisInfo, externallyUsedIdentifiers);

	return std::make_unique<AST>(std::get<Block>(disambiguator(_ast.root())));
}

std::unique_ptr<AST> Program::applyOptimisationSteps(
	Dialect const& _dialect,
	NameDispenser& _nameDispenser,
	std::unique_ptr<AST> _ast,
	std::vector<std::string> const& _optimisationSteps
)
{
	// An empty set of reserved identifiers. It could be a constructor parameter but I don't
	// think it would be useful in this tool. Other tools (like yulopti) have it empty too.
	std::set<YulName> const externallyUsedIdentifiers = {};
	OptimiserStepContext context{
		_dialect,
		_nameDispenser,
		externallyUsedIdentifiers,
		frontend::OptimiserSettings::standard().expectedExecutionsPerDeployment
	};

	auto astRoot = std::get<Block>(ASTCopier{}(_ast->root()));
	for (std::string const& step: _optimisationSteps)
		OptimiserSuite::allSteps().at(step)->run(context, astRoot);

	return std::make_unique<AST>(std::move(astRoot));
}

size_t Program::computeCodeSize(Block const& _ast, CodeWeights const& _weights)
{
	return CodeSize::codeSizeIncludingFunctions(_ast, _weights);
}
