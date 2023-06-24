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

#include <libsolidity/codegen/experimental/Common.h>

#include <libsolidity/analysis/experimental/Analysis.h>
#include <libsolidity/analysis/experimental/TypeInference.h>


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

IRGenerator::IRGenerator(
	EVMVersion _evmVersion,
	std::optional<uint8_t> _eofVersion,
	frontend::RevertStrings, std::map<std::string, unsigned int>,
	DebugInfoSelection const&,
	CharStreamProvider const*,
	Analysis const& _analysis
)
:
m_evmVersion(_evmVersion),
m_eofVersion(_eofVersion),
//		m_debugInfoSelection(_debugInfoSelection),
//		m_soliditySourceProvider(_soliditySourceProvider),
m_env(_analysis.typeSystem().env().clone()),
m_context{_analysis, &m_env, {}, {}}
{
}

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
		auto type = m_context.analysis.annotation<TypeInference>(*_contract.fallbackFunction()).type;
		solAssert(type);
		type = m_context.env->resolve(*type);
		code << IRNames::function(*m_context.env, *_contract.fallbackFunction(), *type) << "()\n";
		m_context.enqueueFunctionDefinition(_contract.fallbackFunction(), *type);
	}
	code << "revert(0,0)\n";
	code << "}\n";

	while (!m_context.functionQueue.empty())
	{
		auto [function, type] = m_context.functionQueue.front();
		m_context.functionQueue.pop_front();
		auto& generatedTypes = m_context.generatedFunctions[function];
		if (!util::contains_if(generatedTypes, [&, type=type](auto _generatedType) { return m_context.env->typeEquals(_generatedType, type); }))
		{
			m_context.generatedFunctions[function].emplace_back(type);
			code << generate(*function, type);
		}
	}

	return code.str();
}

string IRGenerator::generate(FunctionDefinition const& _function, Type _type)
{
	TypeEnvironment newEnv = m_context.env->clone();
	ScopedSaveAndRestore envRestore{m_context.env, &newEnv};
	auto type = m_context.analysis.annotation<TypeInference>(_function).type;
	solAssert(type);
	for (auto err: newEnv.unify(*type, _type))
	{
		solAssert(false, newEnv.typeToString(*type) + " <-> " + newEnv.typeToString(_type));
	}
	std::stringstream code;
	code << "function " << IRNames::function(newEnv, _function, _type) << "(";
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
