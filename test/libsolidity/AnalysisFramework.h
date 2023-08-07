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

#pragma once

#include <test/libsolidity/ErrorCheck.h>

#include <libsolidity/interface/CompilerStack.h>

#include <functional>
#include <string>
#include <memory>

namespace solidity::frontend
{
class Type;
class FunctionType;
using FunctionTypePointer = FunctionType const*;
}

namespace solidity::frontend::test
{

enum class PipelineStage {
	Parsing,
	Analysis,
	Compilation,
};

class AnalysisFramework
{

protected:
	virtual std::pair<SourceUnit const*, langutil::ErrorList>
	parseAnalyseAndReturnError(
		std::string const& _source,
		bool _reportWarnings = false,
		bool _insertLicenseAndVersionPragma = true,
		bool _allowMultipleErrors = false
	);
	virtual ~AnalysisFramework() = default;

	SourceUnit const* parseAndAnalyse(std::string const& _source);
	bool success(std::string const& _source);
	langutil::ErrorList expectError(std::string const& _source, bool _warning = false, bool _allowMultiple = false);

public:
	/// Runs the full compiler pipeline on specified sources. This is the main function of the
	/// framework. Resets the stack, configures it and runs either until the first failed stage or
	/// until the @p _targetStage is reached.
	/// Afterwards the caller can inspect the stack via @p compiler(). The framework provides a few
	/// convenience helpers to check the state and error list, in general the caller can freely
	/// access the stack, including generating outputs if the compilation succeeded.
	bool runFramework(StringMap _sources, PipelineStage _targetStage = PipelineStage::Compilation);
	bool runFramework(std::string _source, PipelineStage _targetStage = PipelineStage::Compilation)
	{
		return runFramework({{"", std::move(_source)}}, _targetStage);
	}

	void resetFramework();

	PipelineStage targetStage() const { return m_targetStage; }
	bool pipelineSuccessful() const { return stageSuccessful(m_targetStage); }
	bool stageSuccessful(PipelineStage _stage) const;

	std::string formatErrors(
		langutil::ErrorList const& _errors,
		bool _colored = false,
		bool _withErrorIds = false
	) const;
	std::string formatError(
		langutil::Error const& _error,
		bool _colored = false,
		bool _withErrorIds = false
	) const;

	static ContractDefinition const* retrieveContractByName(SourceUnit const& _source, std::string const& _name);
	static FunctionTypePointer retrieveFunctionBySignature(
		ContractDefinition const& _contract,
		std::string const& _signature
	);

	/// filter out the warnings in m_warningsToFilter or all warnings and infos if _includeWarningsAndInfos is false
	langutil::ErrorList filterErrors(langutil::ErrorList const& _errorList, bool _includeWarningsAndInfos = true) const;
	langutil::ErrorList filteredErrors(bool _includeWarningsAndInfos = true) const
	{
		return filterErrors(compiler().errors(), _includeWarningsAndInfos);
	}

	/// @returns reference to lazy-instantiated CompilerStack.
	solidity::frontend::CompilerStack& compiler()
	{
		if (!m_compiler)
			m_compiler = createStack();
		return *m_compiler;
	}

	/// @returns reference to lazy-instantiated CompilerStack.
	solidity::frontend::CompilerStack const& compiler() const
	{
		if (!m_compiler)
			m_compiler = createStack();
		return *m_compiler;
	}

protected:
	/// Creates a new instance of @p CompilerStack. Override if your test case needs to pass in
	/// custom constructor arguments.
	virtual std::unique_ptr<CompilerStack> createStack() const;

	/// Configures @p CompilerStack. The default implementation sets basic parameters based on
	/// CLI options. Override if your test case needs extra configuration.
	virtual void setupCompiler(CompilerStack& _compiler);

	/// Executes the requested pipeline stages until @p m_targetStage is reached.
	/// Stops at the first failed stage.
	void executeCompilationPipeline();

	std::vector<std::string> m_warningsToFilter = {"This is a pre-release compiler version"};
	std::vector<std::string> m_messagesToCut = {"Source file requires different compiler version (current compiler is"};

private:
	mutable std::unique_ptr<solidity::frontend::CompilerStack> m_compiler;
	PipelineStage m_targetStage = PipelineStage::Compilation;
};

// Asserts that the compilation down to typechecking
// emits multiple errors of different types and messages, provided in the second argument.
#define CHECK_ALLOW_MULTI(text, expectations) \
do \
{ \
	ErrorList errors = expectError((text), true, true); \
	auto message = searchErrors(errors, (expectations)); \
	BOOST_CHECK_MESSAGE(message.empty(), message); \
} while(0)

#define CHECK_ERROR_OR_WARNING(text, typ, substrings, warning, allowMulti) \
do \
{ \
	ErrorList errors = expectError((text), (warning), (allowMulti)); \
	std::vector<std::pair<Error::Type, std::string>> expectations; \
	for (auto const& str: substrings) \
		expectations.emplace_back((Error::Type::typ), str); \
	auto message = searchErrors(errors, expectations); \
	BOOST_CHECK_MESSAGE(message.empty(), message); \
} while(0)

// [checkError(text, type, substring)] asserts that the compilation down to typechecking
// emits an error of type [type] and with a message containing [substring].
#define CHECK_ERROR(text, type, substring) \
CHECK_ERROR_OR_WARNING(text, type, std::vector<std::string>{(substring)}, false, false)

// [checkError(text, type, substring)] asserts that the compilation down to typechecking
// emits multiple errors of the same type [type] and with a messages containing [substrings].
// Because of the limitations of the preprocessor, you cannot use {{T1, "abc"}, {T2, "def"}} as arguments,
// but have to replace them by (std::vector<std::pair<Error::Type, std::string>>{"abc", "def"})
// (note the parentheses)
#define CHECK_ERROR_ALLOW_MULTI(text, type, substrings) \
CHECK_ERROR_OR_WARNING(text, type, substrings, false, true)

// [checkWarning(text, substring)] asserts that the compilation down to typechecking
// emits a warning and with a message containing [substring].
#define CHECK_WARNING(text, substring) \
CHECK_ERROR_OR_WARNING(text, Warning, std::vector<std::string>{(substring)}, true, false)

// [checkWarningAllowMulti(text, substring)] aserts that the compilation down to typechecking
// emits a warning and with a message containing [substring].
// Because of the limitations of the preprocessor, you cannot use {"abc", "def"} as arguments,
// but have to replace them by (std::vector<std::string>{"abc", "def"}) (note the parentheses)
#define CHECK_WARNING_ALLOW_MULTI(text, substrings) \
CHECK_ERROR_OR_WARNING(text, Warning, substrings, true, true)

// [checkSuccess(text)] asserts that the compilation down to typechecking succeeds.
#define CHECK_SUCCESS(text) do { BOOST_CHECK(success((text))); } while(0)

#define CHECK_SUCCESS_NO_WARNINGS(text) \
do \
{ \
	auto sourceAndError = parseAnalyseAndReturnError((text), true); \
	std::string message; \
	if (!sourceAndError.second.empty()) \
		message = formatErrors(compiler().errors());\
	BOOST_CHECK_MESSAGE(sourceAndError.second.empty(), message); \
} \
while(0)

}
