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
 * @author Alex Beregszaszi
 * @date 2017
 * Component that translates Solidity code into Yul.
 */

#include <libsolidity/codegen/ir/IRGenerator.h>

#include <libsolidity/codegen/ir/IRGeneratorForStatements.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/codegen/ABIFunctions.h>
#include <libsolidity/codegen/CompilerUtils.h>

#include <libyul/AssemblyStack.h>
#include <libyul/Utilities.h>

#include <libdevcore/CommonData.h>
#include <libdevcore/Whiskers.h>
#include <libdevcore/StringUtils.h>

#include <liblangutil/SourceReferenceFormatter.h>

#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace dev;
using namespace dev::solidity;

pair<string, string> IRGenerator::run(ContractDefinition const& _contract)
{
	string const ir = yul::reindent(generate(_contract));

	yul::AssemblyStack asmStack(m_evmVersion, yul::AssemblyStack::Language::StrictAssembly, m_optimiserSettings);
	if (!asmStack.parseAndAnalyze("", ir))
	{
		string errorMessage;
		for (auto const& error: asmStack.errors())
			errorMessage += langutil::SourceReferenceFormatter::formatErrorInformation(*error);
		solAssert(false, ir + "\n\nInvalid IR generated:\n" + errorMessage + "\n");
	}
	asmStack.optimize();

	string warning =
		"/*******************************************************\n"
		" *                       WARNING                       *\n"
		" *  Solidity to Yul compilation is still EXPERIMENTAL  *\n"
		" *       It can result in LOSS OF FUNDS or worse       *\n"
		" *                !USE AT YOUR OWN RISK!               *\n"
		" *******************************************************/\n\n";

	return {warning + ir, warning + asmStack.print()};
}

string IRGenerator::generate(ContractDefinition const& _contract)
{
	solUnimplementedAssert(!_contract.isLibrary(), "Libraries not yet implemented.");

	Whiskers t(R"(
		object "<CreationObject>" {
			code {
				<memoryInit>
				<constructor>
				<deploy>
				<functions>
			}
			object "<RuntimeObject>" {
				code {
					<memoryInit>
					<dispatch>
					<runtimeFunctions>
				}
			}
		}
	)");

	resetContext(_contract);

	t("CreationObject", creationObjectName(_contract));
	t("memoryInit", memoryInit());
	t("constructor", constructorCode(_contract));
	t("deploy", deployCode(_contract));
	// We generate code for all functions and rely on the optimizer to remove them again
	// TODO it would probably be better to only generate functions when internalDispatch or
	// virtualFunctionName is called - same below.
	for (auto const* contract: _contract.annotation().linearizedBaseContracts)
		for (auto const* fun: contract->definedFunctions())
			generateFunction(*fun);
	t("functions", m_context.functionCollector()->requestedFunctions());

	resetContext(_contract);
	m_context.setInheritanceHierarchy(_contract.annotation().linearizedBaseContracts);
	t("RuntimeObject", runtimeObjectName(_contract));
	t("dispatch", dispatchRoutine(_contract));
	for (auto const* contract: _contract.annotation().linearizedBaseContracts)
		for (auto const* fun: contract->definedFunctions())
			generateFunction(*fun);
	t("runtimeFunctions", m_context.functionCollector()->requestedFunctions());
	return t.render();
}

string IRGenerator::generate(Block const& _block)
{
	IRGeneratorForStatements generator(m_context, m_utils);
	_block.accept(generator);
	return generator.code();
}

string IRGenerator::generateFunction(FunctionDefinition const& _function)
{
	string functionName = m_context.functionName(_function);
	return m_context.functionCollector()->createFunction(functionName, [&]() {
		Whiskers t(R"(
			function <functionName>(<params>) <returns> {
				for { let return_flag := 1 } return_flag {} {
					<body>
					break
				}
			}
		)");
		t("functionName", functionName);
		string params;
		for (auto const& varDecl: _function.parameters())
			params += (params.empty() ? "" : ", ") + m_context.addLocalVariable(*varDecl);
		t("params", params);
		string retParams;
		for (auto const& varDecl: _function.returnParameters())
			retParams += (retParams.empty() ? "" : ", ") + m_context.addLocalVariable(*varDecl);
		t("returns", retParams.empty() ? "" : " -> " + retParams);
		t("body", generate(_function.body()));
		return t.render();
	});
}

string IRGenerator::generateGetter(VariableDeclaration const& _varDecl)
{
	string functionName = m_context.functionName(_varDecl);

	Type const* type = _varDecl.annotation().type;

	solAssert(!_varDecl.isConstant(), "");
	solAssert(_varDecl.isStateVariable(), "");

	solUnimplementedAssert(type->isValueType(), "");

	return m_context.functionCollector()->createFunction(functionName, [&]() {
		pair<u256, unsigned> slot_offset = m_context.storageLocationOfVariable(_varDecl);

		return Whiskers(R"(
			function <functionName>() -> rval {
				rval := <readStorage>(<slot>)
			}
		)")
		("functionName", functionName)
		("readStorage", m_utils.readFromStorage(*type, slot_offset.second, false))
		("slot", slot_offset.first.str())
		.render();
	});
}

string IRGenerator::constructorCode(ContractDefinition const& _contract)
{
	// TODO initialize state variables in base to derived order.
	// TODO base constructors
	// TODO callValueCheck if there is no constructor.
	if (FunctionDefinition const* constructor = _contract.constructor())
	{
		string out;
		if (!constructor->isPayable())
			out = callValueCheck();
		solUnimplementedAssert(constructor->parameters().empty(), "");
		return move(out) + m_context.functionName(*constructor) + "()\n";
	}

	return {};
}

string IRGenerator::deployCode(ContractDefinition const& _contract)
{
	Whiskers t(R"X(
		codecopy(0, dataoffset("<object>"), datasize("<object>"))
		return(0, datasize("<object>"))
	)X");
	t("object", runtimeObjectName(_contract));
	return t.render();
}

string IRGenerator::callValueCheck()
{
	return "if callvalue() { revert(0, 0) }";
}

string IRGenerator::creationObjectName(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + to_string(_contract.id());
}

string IRGenerator::runtimeObjectName(ContractDefinition const& _contract)
{
	return _contract.name() + "_" + to_string(_contract.id()) + "_deployed";
}

string IRGenerator::dispatchRoutine(ContractDefinition const& _contract)
{
	Whiskers t(R"X(
		if iszero(lt(calldatasize(), 4))
		{
			let selector := <shr224>(calldataload(0))
			switch selector
			<#cases>
			case <functionSelector>
			{
				// <functionName>
				<callValueCheck>
				<assignToParams> <abiDecode>(4, calldatasize())
				<assignToRetParams> <function>(<params>)
				let memPos := <allocate>(0)
				let memEnd := <abiEncode>(memPos <comma> <retParams>)
				return(memPos, sub(memEnd, memPos))
			}
			</cases>
			default {}
		}
		<fallback>
	)X");
	t("shr224", m_utils.shiftRightFunction(224));
	vector<map<string, string>> functions;
	for (auto const& function: _contract.interfaceFunctions())
	{
		functions.push_back({});
		map<string, string>& templ = functions.back();
		templ["functionSelector"] = "0x" + function.first.hex();
		FunctionTypePointer const& type = function.second;
		templ["functionName"] = type->externalSignature();
		templ["callValueCheck"] = type->isPayable() ? "" : callValueCheck();

		unsigned paramVars = make_shared<TupleType>(type->parameterTypes())->sizeOnStack();
		unsigned retVars = make_shared<TupleType>(type->returnParameterTypes())->sizeOnStack();
		templ["assignToParams"] = paramVars == 0 ? "" : "let " + suffixedVariableNameList("param_", 0, paramVars) + " := ";
		templ["assignToRetParams"] = retVars == 0 ? "" : "let " + suffixedVariableNameList("ret_", 0, retVars) + " := ";

		ABIFunctions abiFunctions(m_evmVersion, m_context.functionCollector());
		templ["abiDecode"] = abiFunctions.tupleDecoder(type->parameterTypes());
		templ["params"] = suffixedVariableNameList("param_", 0, paramVars);
		templ["retParams"] = suffixedVariableNameList("ret_", retVars, 0);

		if (FunctionDefinition const* funDef = dynamic_cast<FunctionDefinition const*>(&type->declaration()))
			templ["function"] = generateFunction(*funDef);
		else if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(&type->declaration()))
			templ["function"] = generateGetter(*varDecl);
		else
			solAssert(false, "Unexpected declaration for function!");

		templ["allocate"] = m_utils.allocationFunction();
		templ["abiEncode"] = abiFunctions.tupleEncoder(type->returnParameterTypes(), type->returnParameterTypes(), false);
		templ["comma"] = retVars == 0 ? "" : ", ";
	}
	t("cases", functions);
	if (FunctionDefinition const* fallback = _contract.fallbackFunction())
	{
		string fallbackCode;
		if (!fallback->isPayable())
			fallbackCode += callValueCheck();
		fallbackCode += generateFunction(*fallback) + "() stop()";

		t("fallback", fallbackCode);
	}
	else
		t("fallback", "revert(0, 0)");
	return t.render();
}

string IRGenerator::memoryInit()
{
	// This function should be called at the beginning of the EVM call frame
	// and thus can assume all memory to be zero, including the contents of
	// the "zero memory area" (the position CompilerUtils::zeroPointer points to).
	return
		Whiskers{"mstore(<memPtr>, <generalPurposeStart>)"}
		("memPtr", to_string(CompilerUtils::freeMemoryPointer))
		("generalPurposeStart", to_string(CompilerUtils::generalPurposeMemoryStart))
		.render();
}

void IRGenerator::resetContext(ContractDefinition const& _contract)
{
	solAssert(
		m_context.functionCollector()->requestedFunctions().empty(),
		"Reset context while it still had functions."
	);
	m_context = IRGenerationContext(m_evmVersion, m_optimiserSettings);
	m_utils = YulUtilFunctions(m_evmVersion, m_context.functionCollector());

	m_context.setInheritanceHierarchy(_contract.annotation().linearizedBaseContracts);
	for (auto const& var: ContractType(_contract).stateVariables())
		m_context.addStateVariable(*get<0>(var), get<1>(var), get<2>(var));
}
