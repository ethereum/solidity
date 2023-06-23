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

#include <libsolidity/codegen/experimental/IRGenerator.h>

#include <libsolidity/codegen/experimental/IRGenerationContext.h>
#include <libsolidity/codegen/experimental/IRGeneratorForStatements.h>

#include <libsolidity/codegen/ir/Common.h>

#include <libyul/YulStack.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/optimiser/ASTCopier.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/Whiskers.h>

#include <range/v3/view/drop_last.hpp>

#include <variant>

using namespace std;
using namespace solidity;
using namespace solidity::frontend::experimental;
using namespace solidity::langutil;
using namespace solidity::util;

string IRGenerator::run(
	ContractDefinition const& _contract,
	bytes const& /*_cborMetadata*/,
	map<ContractDefinition const*, string_view const> const& /*_otherYulSources*/
)
{

	Whiskers t(R"(
		object "<CreationObject>" {
			code {
				codecopy(0, dataoffset("<DeployedObject>"), datasize("<DeployedObject>"))
				return(0, datasize("<DeployedObject>"))
			}
			object "<DeployedObject>" {
				code {
					<code>
				}
			}
		}
	)");
	t("CreationObject", IRNames::creationObject(_contract));
	t("DeployedObject", IRNames::deployedObject(_contract));
	t("code", generate(_contract));

	return t.render();
}

string IRGenerator::generate(ContractDefinition const& _contract)
{
	std::stringstream code;
	code << "{\n";
	if (_contract.fallbackFunction())
	{
		code << IRNames::function(*_contract.fallbackFunction()) << "()\n";
		m_context.functionQueue.emplace_front(_contract.fallbackFunction());
	}
	code << "revert(0,0)\n";
	code << "}\n";

	while (!m_context.functionQueue.empty())
	{
		FunctionDefinition const* function = m_context.functionQueue.front();
		m_context.functionQueue.pop_front();
		if (!m_context.generatedFunctions.count(function))
		{
			m_context.generatedFunctions.insert(function);
			code << generate(*function);
		}
	}

	return code.str();
}

string IRGenerator::generate(FunctionDefinition const& _function)
{
	std::stringstream code;
	code << "function " << IRNames::function(_function) << "(";
	if (_function.parameters().size() > 1)
		for (auto const& arg: _function.parameters() | ranges::views::drop_last(1))
			code << IRNames::localVariable(*arg) << ", ";
	if (!_function.parameters().empty())
		code << IRNames::localVariable(*_function.parameters().back());
	code << ")";
	if (_function.returnParameterList() && !_function.returnParameters().empty())
	{
		code << " -> ";
		if (_function.returnParameters().size() > 1)
			for (auto const& arg: _function.returnParameters() | ranges::views::drop_last(1))
				code << IRNames::localVariable(*arg) << ", ";
		if (!_function.returnParameters().empty())
			code << IRNames::localVariable(*_function.returnParameters().back());
	}
	code << "{\n";
	for (auto _statement: _function.body().statements())
	{
		IRGeneratorForStatements statementGenerator{m_context};
		code << statementGenerator.generate(*_statement);
	}
	code << "}\n";
	return code.str();
}
