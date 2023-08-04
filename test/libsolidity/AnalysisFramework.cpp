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
/**
 * Framework for testing features from the analysis phase of compiler.
 */

#include <test/libsolidity/AnalysisFramework.h>

#include <test/libsolidity/util/Common.h>
#include <test/libsolidity/util/SoltestErrors.h>
#include <test/Common.h>

#include <libsolidity/interface/CompilerStack.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolidity/ast/AST.h>

#include <liblangutil/Scanner.h>

#include <libsolutil/FunctionSelector.h>

#include <boost/test/unit_test.hpp>

using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

std::pair<SourceUnit const*, ErrorList> AnalysisFramework::runAnalysisAndExpectNoParsingErrors(
	std::string const& _source,
	bool _includeWarningsAndInfos,
	bool _addPreamble,
	bool _allowMultiple
)
{
	runFramework(_addPreamble ? withPreamble(_source) : _source, PipelineStage::Analysis);

	if (!stageSuccessful(PipelineStage::Parsing))
		BOOST_FAIL("Parsing contract failed in analysis test suite:" + formatErrors(m_compiler->errors()));

	ErrorList errors = filteredErrors(_includeWarningsAndInfos);
	if (errors.size() > 1 && !_allowMultiple)
		BOOST_FAIL("Multiple errors found: " + formatErrors(errors));

	return make_pair(&compiler().ast(""), std::move(errors));
}

bool AnalysisFramework::runFramework(StringMap _sources, PipelineStage _targetStage)
{
	resetFramework();
	m_targetStage = _targetStage;
	soltestAssert(m_compiler);

	m_compiler->setSources(std::move(_sources));
	setupCompiler(*m_compiler);
	executeCompilationPipeline();
	return pipelineSuccessful();
}

void AnalysisFramework::resetFramework()
{
	compiler().reset();
	m_targetStage = PipelineStage::Compilation;
}

std::unique_ptr<CompilerStack> AnalysisFramework::createStack() const
{
	return std::make_unique<CompilerStack>();
}

void AnalysisFramework::setupCompiler(CompilerStack& _compiler)
{
	_compiler.setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
}

void AnalysisFramework::executeCompilationPipeline()
{
	soltestAssert(m_compiler);

	// If you add a new stage, remember to handle it below.
	soltestAssert(
		m_targetStage == PipelineStage::Parsing ||
		m_targetStage == PipelineStage::Analysis ||
		m_targetStage == PipelineStage::Compilation
	);

	bool parsingSuccessful = m_compiler->parse();
	soltestAssert(parsingSuccessful || !filteredErrors(false /* _includeWarningsAndInfos */).empty());
	if (!parsingSuccessful || stageSuccessful(m_targetStage))
		return;

	bool analysisSuccessful = m_compiler->analyze();
	soltestAssert(analysisSuccessful || !filteredErrors(false /* _includeWarningsAndInfos */).empty());
	if (!analysisSuccessful || stageSuccessful(m_targetStage))
		return;

	bool compilationSuccessful = m_compiler->compile();
	soltestAssert(compilationSuccessful || !filteredErrors(false /* _includeWarningsAndInfos */).empty());
	soltestAssert(stageSuccessful(m_targetStage) == compilationSuccessful);
}

ErrorList AnalysisFramework::filterErrors(ErrorList const& _errorList, bool _includeWarningsAndInfos) const
{
	ErrorList errors;
	for (auto const& currentError: _errorList)
	{
		solAssert(currentError->comment(), "");
		if (!Error::isError(currentError->type()))
		{
			if (!_includeWarningsAndInfos)
				continue;
			bool ignoreWarningsAndInfos = false;
			for (auto const& filter: m_warningsToFilter)
				if (currentError->comment()->find(filter) == 0)
				{
					ignoreWarningsAndInfos = true;
					break;
				}
			if (ignoreWarningsAndInfos)
				continue;
		}

		std::shared_ptr<Error const> newError = currentError;
		for (auto const& messagePrefix: m_messagesToCut)
			if (currentError->comment()->find(messagePrefix) == 0)
			{
				SourceLocation const* location = currentError->sourceLocation();
				// sufficient for now, but in future we might clone the error completely, including the secondary location
				newError = std::make_shared<Error>(
					currentError->errorId(),
					currentError->type(),
					messagePrefix + " ....",
					location ? *location : SourceLocation()
				);
				break;
			}

		errors.emplace_back(newError);
	}

	return errors;
}

bool AnalysisFramework::stageSuccessful(PipelineStage _stage) const
{
	switch (_stage) {
		case PipelineStage::Parsing: return compiler().state() >= CompilerStack::Parsed;
		case PipelineStage::Analysis: return compiler().state() >= CompilerStack::AnalysisSuccessful;
		case PipelineStage::Compilation: return compiler().state() >= CompilerStack::CompilationSuccessful;
	}
	unreachable();
}

ErrorList AnalysisFramework::runAnalysisAndExpectError(
	std::string const& _source,
	bool _includeWarningsAndInfos,
	bool _allowMultiple
)
{
	auto [ast, errors] = runAnalysisAndExpectNoParsingErrors(_source, _includeWarningsAndInfos, true, _allowMultiple);
	BOOST_REQUIRE(!errors.empty());
	BOOST_REQUIRE_MESSAGE(ast, "Expected error, but no error happened.");
	return errors;
}

std::string AnalysisFramework::formatErrors(
	langutil::ErrorList const& _errors,
	bool _colored,
	bool _withErrorIds
) const
{
	return SourceReferenceFormatter::formatErrorInformation(
		_errors,
		*m_compiler,
		_colored,
		_withErrorIds
	);
}

std::string AnalysisFramework::formatError(
	Error const& _error,
	bool _colored,
	bool _withErrorIds
) const
{
	return SourceReferenceFormatter::formatErrorInformation(
		_error,
		*m_compiler,
		_colored,
		_withErrorIds
	);
}

ContractDefinition const* AnalysisFramework::retrieveContractByName(SourceUnit const& _source, std::string const& _name)
{
	ContractDefinition* contract = nullptr;

	for (std::shared_ptr<ASTNode> const& node: _source.nodes())
		if ((contract = dynamic_cast<ContractDefinition*>(node.get())) && contract->name() == _name)
			return contract;

	return nullptr;
}

FunctionTypePointer AnalysisFramework::retrieveFunctionBySignature(
	ContractDefinition const& _contract,
	std::string const& _signature
)
{
	return _contract.interfaceFunctions()[util::selectorFromSignatureH32(_signature)];
}
