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

#include <libsolidity/experimental/codegen/IRGenerator.h>
#include <libsolidity/experimental/codegen/IRGenerationContext.h>
#include <libsolidity/experimental/codegen/IRGeneratorForStatements.h>

#include <libsolidity/experimental/codegen/Common.h>

#include <libsolidity/experimental/analysis/Analysis.h>
#include <libsolidity/experimental/analysis/TypeInference.h>

#include <libsolidity/experimental/ast/TypeSystemHelper.h>

#include <libyul/AsmPrinter.h>
#include <libyul/AST.h>
#include <libyul/optimiser/ASTCopier.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <libsolutil/Whiskers.h>

#include <range/v3/view/drop_last.hpp>

#include <variant>

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
):
	m_evmVersion(_evmVersion),
	m_eofVersion(_eofVersion),
	//m_debugInfoSelection(_debugInfoSelection),
	//m_soliditySourceProvider(_soliditySourceProvider),
	m_env(_analysis.typeSystem().env().clone()),
	m_context{_analysis, &m_env, {}, {}}
{
}

std::string IRGenerator::run(
	ContractDefinition const& _contract,
	bytes const& /*_cborMetadata*/,
	std::map<ContractDefinition const*, std::string_view const> const& /*_otherYulSources*/
)
{
	solUnimplementedAssert(!m_eofVersion.has_value(), "Experimental IRGenerator not implemented for EOF");

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

std::string IRGenerator::generate(ContractDefinition const& _contract)
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
		auto queueEntry = m_context.functionQueue.front();
		m_context.functionQueue.pop_front();
		auto& generatedTypes = m_context.generatedFunctions.insert(std::make_pair(queueEntry.function, std::vector<Type>{})).first->second;
		if (!util::contains_if(generatedTypes, [&](auto const& _generatedType) { return m_context.env->typeEquals(_generatedType, queueEntry.type); }))
		{
			generatedTypes.emplace_back(queueEntry.type);
			code << generate(*queueEntry.function, queueEntry.type);
		}
	}

	return code.str();
}

std::string IRGenerator::generate(FunctionDefinition const& _function, Type _type)
{
	TypeEnvironment newEnv = m_context.env->clone();
	ScopedSaveAndRestore envRestore{m_context.env, &newEnv};
	auto type = m_context.analysis.annotation<TypeInference>(_function).type;
	solAssert(type);
	for (auto err: newEnv.unify(*type, _type))
	{
		TypeEnvironmentHelpers helper{newEnv};
		solAssert(false, helper.typeToString(*type) + " <-> " + helper.typeToString(_type));
	}
	std::stringstream code;
	code << "function " << IRNames::function(newEnv, _function, _type) << "(";
	if (_function.parameters().size() > 1)
		for (auto const& arg: _function.parameters() | ranges::views::drop_last(1))
			code << IRNames::localVariable(*arg) << ", ";
	if (!_function.parameters().empty())
		code << IRNames::localVariable(*_function.parameters().back());
	code << ")";
	if (_function.experimentalReturnExpression())
	{
		auto returnType = m_context.analysis.annotation<TypeInference>(*_function.experimentalReturnExpression()).type;
		solAssert(returnType);
		if (!m_env.typeEquals(*returnType, m_context.analysis.typeSystem().type(PrimitiveType::Unit, {})))
		{
			// TODO: destructure tuples.
			code << " -> " << IRNames::localVariable(*_function.experimentalReturnExpression()) << " ";
		}
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
