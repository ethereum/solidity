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
	virtual std::pair<SourceUnit const*, std::shared_ptr<Error const>>
	parseAnalyseAndReturnError(
		std::string const& _source,
		bool _reportWarnings = false,
		bool _insertVersionPragma = true,
		bool _allowMultipleErrors = false
	);

	SourceUnit const* parseAndAnalyse(std::string const& _source);
	bool success(std::string const& _source);
	Error expectError(std::string const& _source, bool _warning = false, bool _allowMultiple = false);

	std::string formatErrors();
	std::string formatError(Error const& _error);

	static ContractDefinition const* retrieveContractByName(SourceUnit const& _source, std::string const& _name);
	static FunctionTypePointer retrieveFunctionBySignature(
		ContractDefinition const& _contract,
		std::string const& _signature
	);

	std::vector<std::string> m_warningsToFilter = {"This is a pre-release compiler version"};
	dev::solidity::CompilerStack m_compiler;
};


#define CHECK_ERROR_OR_WARNING(text, typ, substring, warning, allowMulti) \
do \
{ \
	Error err = expectError((text), (warning), (allowMulti)); \
	BOOST_CHECK(err.type() == (Error::Type::typ)); \
	BOOST_CHECK(searchErrorMessage(err, (substring))); \
} while(0)

// [checkError(text, type, substring)] asserts that the compilation down to typechecking
// emits an error of type [type] and with a message containing [substring].
#define CHECK_ERROR(text, type, substring) \
CHECK_ERROR_OR_WARNING(text, type, substring, false, false)

// [checkError(text, type, substring)] asserts that the compilation down to typechecking
// emits an error of type [type] and with a message containing [substring].
#define CHECK_ERROR_ALLOW_MULTI(text, type, substring) \
CHECK_ERROR_OR_WARNING(text, type, substring, false, true)

// [checkWarning(text, substring)] asserts that the compilation down to typechecking
// emits a warning and with a message containing [substring].
#define CHECK_WARNING(text, substring) \
CHECK_ERROR_OR_WARNING(text, Warning, substring, true, false)

// [checkWarningAllowMulti(text, substring)] aserts that the compilation down to typechecking
// emits a warning and with a message containing [substring].
#define CHECK_WARNING_ALLOW_MULTI(text, substring) \
CHECK_ERROR_OR_WARNING(text, Warning, substring, true, true)

// [checkSuccess(text)] asserts that the compilation down to typechecking succeeds.
#define CHECK_SUCCESS(text) do { BOOST_CHECK(success((text))); } while(0)

#define CHECK_SUCCESS_NO_WARNINGS(text) \
do \
{ \
	auto sourceAndError = parseAnalyseAndReturnError((text), true); \
	std::string message; \
	if (sourceAndError.second) \
		message = formatError(*sourceAndError.second); \
	BOOST_CHECK_MESSAGE(!sourceAndError.second, message); \
} \
while(0)

}
}
}
