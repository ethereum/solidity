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

#include <libsolidity/interface/CompilerStack.h>
#include <libsolidity/interface/SourceReferenceFormatter.h>

#include <libsolidity/ast/AST.h>

#include <libdevcore/SHA3.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::test;

pair<SourceUnit const*, shared_ptr<Error const>>
AnalysisFramework::parseAnalyseAndReturnError(
	string const& _source,
	bool _reportWarnings,
	bool _insertVersionPragma,
	bool _allowMultipleErrors
)
{
	m_compiler.reset();
	m_compiler.addSource("", _insertVersionPragma ? "pragma solidity >=0.0;\n" + _source : _source);
	if (!m_compiler.parse())
	{
		printErrors();
		BOOST_ERROR("Parsing contract failed in analysis test suite.");
	}

	m_compiler.analyze();

	std::shared_ptr<Error const> firstError;
	for (auto const& currentError: m_compiler.errors())
	{
		solAssert(currentError->comment(), "");
		if (currentError->comment()->find("This is a pre-release compiler version") == 0)
			continue;

		if (_reportWarnings || (currentError->type() != Error::Type::Warning))
		{
			if (firstError && !_allowMultipleErrors)
			{
				printErrors();
				BOOST_FAIL("Multiple errors found.");
			}
			if (!firstError)
				firstError = currentError;
		}
	}

	return make_pair(&m_compiler.ast(), firstError);
}

SourceUnit const* AnalysisFramework::parseAndAnalyse(string const& _source)
{
	auto sourceAndError = parseAnalyseAndReturnError(_source);
	BOOST_REQUIRE(!!sourceAndError.first);
	BOOST_REQUIRE(!sourceAndError.second);
	return sourceAndError.first;
}

bool AnalysisFramework::success(string const& _source)
{
	return !parseAnalyseAndReturnError(_source).second;
}

Error AnalysisFramework::expectError(std::string const& _source, bool _warning, bool _allowMultiple)
{
	auto sourceAndError = parseAnalyseAndReturnError(_source, _warning, true, _allowMultiple);
	BOOST_REQUIRE(!!sourceAndError.second);
	BOOST_REQUIRE(!!sourceAndError.first);
	return *sourceAndError.second;
}

void AnalysisFramework::printErrors()
{
	for (auto const& error: m_compiler.errors())
		SourceReferenceFormatter::printExceptionInformation(
			std::cerr,
			*error,
			(error->type() == Error::Type::Warning) ? "Warning" : "Error",
			[&](std::string const& _sourceName) -> solidity::Scanner const& { return m_compiler.scanner(_sourceName); }
		);
}

ContractDefinition const* AnalysisFramework::retrieveContract(SourceUnit const& _source, unsigned index)
{
	ContractDefinition* contract = nullptr;
	unsigned counter = 0;
	for (shared_ptr<ASTNode> const& node: _source.nodes())
		if ((contract = dynamic_cast<ContractDefinition*>(node.get())) && counter == index)
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
