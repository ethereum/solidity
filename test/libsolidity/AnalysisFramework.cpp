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

#include <test/libsolidity/AnalysisFramework.h>

#include <test/Options.h>

#include <libsolidity/interface/CompilerStack.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolidity/ast/AST.h>

#include <liblangutil/Scanner.h>

#include <libdevcore/Keccak256.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::solidity;
using namespace dev::solidity::test;

pair<SourceUnit const*, ErrorList>
AnalysisFramework::parseAnalyseAndReturnError(
	string const& _source,
	bool _reportWarnings,
	bool _insertVersionPragma,
	bool _allowMultipleErrors
)
{
	m_compiler.reset();
	m_compiler.addSource("", _insertVersionPragma ? "pragma solidity >=0.0;\n" + _source : _source);
	m_compiler.setEVMVersion(dev::test::Options::get().evmVersion());
	if (!m_compiler.parse())
	{
		BOOST_FAIL("Parsing contract failed in analysis test suite:" + formatErrors());
	}

	m_compiler.analyze();

	ErrorList errors = filterErrors(m_compiler.errors(), _reportWarnings);
	if (errors.size() > 1 && !_allowMultipleErrors)
		BOOST_FAIL("Multiple errors found: " + formatErrors());

	return make_pair(&m_compiler.ast(""), std::move(errors));
}

ErrorList AnalysisFramework::filterErrors(ErrorList const& _errorList, bool _includeWarnings) const
{
	ErrorList errors;
	for (auto const& currentError: _errorList)
	{
		solAssert(currentError->comment(), "");
		if (currentError->type() == Error::Type::Warning)
		{
			if (!_includeWarnings)
				continue;
			bool ignoreWarning = false;
			for (auto const& filter: m_warningsToFilter)
				if (currentError->comment()->find(filter) == 0)
				{
					ignoreWarning = true;
					break;
				}
			if (ignoreWarning)
				continue;
		}

		errors.emplace_back(currentError);
	}

	return errors;
}

SourceUnit const* AnalysisFramework::parseAndAnalyse(string const& _source)
{
	auto sourceAndError = parseAnalyseAndReturnError(_source);
	BOOST_REQUIRE(!!sourceAndError.first);
	string message;
	if (!sourceAndError.second.empty())
		message = "Unexpected error: " + formatErrors();
	BOOST_REQUIRE_MESSAGE(sourceAndError.second.empty(), message);
	return sourceAndError.first;
}

bool AnalysisFramework::success(string const& _source)
{
	return parseAnalyseAndReturnError(_source).second.empty();
}

ErrorList AnalysisFramework::expectError(std::string const& _source, bool _warning, bool _allowMultiple)
{
	auto sourceAndErrors = parseAnalyseAndReturnError(_source, _warning, true, _allowMultiple);
	BOOST_REQUIRE(!sourceAndErrors.second.empty());
	BOOST_REQUIRE_MESSAGE(!!sourceAndErrors.first, "Expected error, but no error happened.");
	return sourceAndErrors.second;
}

string AnalysisFramework::formatErrors() const
{
	string message;
	for (auto const& error: m_compiler.errors())
		message += formatError(*error);
	return message;
}

string AnalysisFramework::formatError(Error const& _error) const
{
	return SourceReferenceFormatter::formatExceptionInformation(
			_error,
			(_error.type() == Error::Type::Warning) ? "Warning" : "Error"
		);
}

ContractDefinition const* AnalysisFramework::retrieveContractByName(SourceUnit const& _source, string const& _name)
{
	ContractDefinition* contract = nullptr;

	for (shared_ptr<ASTNode> const& node: _source.nodes())
		if ((contract = dynamic_cast<ContractDefinition*>(node.get())) && contract->name() == _name)
			return contract;

	return nullptr;
}

FunctionTypePointer AnalysisFramework::retrieveFunctionBySignature(
	ContractDefinition const& _contract,
	std::string const& _signature
)
{
	FixedHash<4> hash(dev::keccak256(_signature));
	return _contract.interfaceFunctions()[hash];
}
