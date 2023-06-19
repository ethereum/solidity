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

#include <libsolidity/codegen/experimental/IRGeneratorForStatements.h>

#include <libsolidity/codegen/ir/Common.h>

#include <libyul/YulStack.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/optimiser/ASTCopier.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/Whiskers.h>

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
) const
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

string IRGenerator::generate(ContractDefinition const& _contract) const
{
	std::stringstream code;
	code << "{\n";
	if (_contract.fallbackFunction())
	{
		code << IRNames::function(*_contract.fallbackFunction()) << "()\n";
	}
	code << "revert(0,0)\n";
	code << "}\n";

	for (FunctionDefinition const* f: _contract.definedFunctions())
		code << generate(*f);

	return code.str();
}

string IRGenerator::generate(FunctionDefinition const& _function) const
{
	std::stringstream code;
	code << "function " << IRNames::function(_function) << "() {\n";
	for (auto _statement: _function.body().statements())
	{
		IRGeneratorForStatements statementGenerator{m_analysis};
		code << statementGenerator.generate(*_statement);
	}
	code << "}\n";
	return code.str();
}
