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
/**
 * Framework for testing features from the analysis phase of compiler.
 */

#pragma once

#include <test/libsolidity/ErrorCheck.h>

#include <libsolidity/interface/CompilerStack.h>

#include <functional>
#include <string>
#include <memory>

namespace dev
{
namespace solidity
{

class Type;
class FunctionType;
using TypePointer = std::shared_ptr<Type const>;
using FunctionTypePointer = std::shared_ptr<FunctionType const>;

namespace test
{

class AnalysisFramework
{

protected:
	virtual std::pair<SourceUnit const*, langutil::ErrorList>
	parseAnalyseAndReturnError(
		std::string const& _source,
		bool _reportWarnings = false,
		bool _insertVersionPragma = true,
		bool _allowMultipleErrors = false
	);
	virtual ~AnalysisFramework() = default;

	SourceUnit const* parseAndAnalyse(std::string const& _source);
	bool success(std::string const& _source);
	langutil::ErrorList expectError(std::string const& _source, bool _warning = false, bool _allowMultiple = false);

	std::string formatErrors() const;
	std::string formatError(langutil::Error const& _error) const;

	static ContractDefinition const* retrieveContractByName(SourceUnit const& _source, std::string const& _name);
	static FunctionTypePointer retrieveFunctionBySignature(
		ContractDefinition const& _contract,
		std::string const& _signature
	);

	// filter out the warnings in m_warningsToFilter or all warnings if _includeWarnings is false
	langutil::ErrorList filterErrors(langutil::ErrorList const& _errorList, bool _includeWarnings) const;

	std::vector<std::string> m_warningsToFilter = {"This is a pre-release compiler version"};

	/// @returns reference to lazy-instanciated CompilerStack.
	dev::solidity::CompilerStack& compiler()
	{
		if (!m_compiler)
			m_compiler = std::make_unique<dev::solidity::CompilerStack>();
		return *m_compiler;
	}

	/// @returns reference to lazy-instanciated CompilerStack.
	dev::solidity::CompilerStack const& compiler() const
	{
		if (!m_compiler)
			m_compiler = std::make_unique<dev::solidity::CompilerStack>();
		return *m_compiler;
	}

private:
	mutable std::unique_ptr<dev::solidity::CompilerStack> m_compiler;
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
		message = formatErrors();\
	BOOST_CHECK_MESSAGE(sourceAndError.second.empty(), message); \
} \
while(0)

}
}
}
