// SPDX-License-Identifier: GPL-3.0
/**
 * Framework for testing features from the analysis phase of compiler.
 */

#include <test/libsolidity/AnalysisFramework.h>

#include <test/Common.h>

#include <libsolidity/interface/CompilerStack.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolidity/ast/AST.h>

#include <liblangutil/Scanner.h>

#include <libsolutil/Keccak256.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace solidity;
using namespace solidity::util;
using namespace solidity::langutil;
using namespace solidity::frontend;
using namespace solidity::frontend::test;

pair<SourceUnit const*, ErrorList>
AnalysisFramework::parseAnalyseAndReturnError(
	string const& _source,
	bool _reportWarnings,
	bool _insertLicenseAndVersionPragma,
	bool _allowMultipleErrors,
	bool _allowRecoveryErrors
)
{
	compiler().reset();
	compiler().setSources({{"",
		_insertLicenseAndVersionPragma ?
		"pragma solidity >=0.0;\n// SPDX-License-Identifier: GPL-3.0\n" + _source :
		_source
	}});
	compiler().setEVMVersion(solidity::test::CommonOptions::get().evmVersion());
	compiler().setParserErrorRecovery(_allowRecoveryErrors);
	_allowMultipleErrors = _allowMultipleErrors || _allowRecoveryErrors;
	if (!compiler().parse())
	{
		BOOST_FAIL("Parsing contract failed in analysis test suite:" + formatErrors());
	}

	compiler().analyze();

	ErrorList errors = filterErrors(compiler().errors(), _reportWarnings);
	if (errors.size() > 1 && !_allowMultipleErrors)
		BOOST_FAIL("Multiple errors found: " + formatErrors());

	return make_pair(&compiler().ast(""), std::move(errors));
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
	for (auto const& error: compiler().errors())
		message += formatError(*error);
	return message;
}

string AnalysisFramework::formatError(Error const& _error) const
{
	return SourceReferenceFormatter::formatErrorInformation(_error);
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
	FixedHash<4> hash(util::keccak256(_signature));
	return _contract.interfaceFunctions()[hash];
}
